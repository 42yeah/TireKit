//
//  main.cpp
//  TireKit
//
//  Created by apple on 2019/5/9.
//  Copyright © 2019 aiofwa. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <AppCore/AppCore.h>
#include <AppCore/Overlay.h>
#include <AppCore/JSHelpers.h>
#include <AppCore/Window.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <thread>
#include <unistd.h>

using namespace ultralight;

class MyApp : public LoadListener {
public:
    String loadFile(String name) {
        std::ifstream reader(name.utf8().data());
        std::stringstream stream;
        stream << reader.rdbuf();
        std::string str = stream.str();
        String content(str.c_str(), str.length());
        
        return content;
    }
    
    MyApp(Ref<Window> win) {
        overlay = Overlay::Create(win, win->width(), win->height(), 0, 0);
        overlay->view()->set_load_listener(this);
        overlay->view()->LoadURL(("file:///Res/index.html"));
    }
    
    virtual void OnDOMReady(View *caller) {
        SetJSContext(caller->js_context());
        JSObject global = JSGlobalObject();
        std::cout << "Dom's ready." << std::endl;
        global["setupServer"] = BindJSCallback(&MyApp::setupServer);
        global["stopServer"] = BindJSCallback(&MyApp::stopServer);
        global["showPane"] = BindJSCallback(&MyApp::showPane);
        global["setDelay"] = BindJSCallback(&MyApp::setDelay);
    }
    
    void setupServer(const JSObject& thisObject, const JSArgs& args) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        auto port = JSEval("document.getElementById('port').value").ToInteger();
        struct sockaddr_in sin;
        
        sin.sin_addr.s_addr = INADDR_ANY;
        sin.sin_port = htons(port);
        sin.sin_family = AF_INET;
        int result = bind(sock, (struct sockaddr *) &sin, sizeof(sin));
        if (result < 0) {
            JSEval("document.getElementById('err').innerHTML = '服务器架设失败。换个端口试试吧。'");
        } else {
            std::string msg = std::string("header('服务器架设在了 ") + std::to_string(port) + std::string(" 了。')");
            JSString cmd(msg.c_str());
            JSEval(cmd);
            JSEval("pane('sendPane')");
        }
        ::listen(sock, 5);
        
        running = true;
        std::thread listenThread(std::bind(&MyApp::listen, this));
        listenThread.detach();
    }

    void stopServer(const JSObject &thisObject, const JSArgs &args) {
        ok = false;
        running = false;
        close(sock);
        overlay->view()->LoadURL(("file:///Res/index.html"));
    }
    
    void showPane(const JSObject &thisObject, const JSArgs &args) {
        auto pane = (String) args[0].ToString();
        if (running) {
            std::string cmd = std::string("pane('") + std::string(pane.utf8().data()) + "')";
            JSEval(cmd.c_str());
        }
    }
    
    void setDelay(const JSObject &thisObject, const JSArgs &args) {
        auto input = JSEval("document.getElementById('delay').value");
        delay = (int) input.ToInteger();
    }
    
    int waitForConnection() {
        auto client = accept(sock, nullptr, nullptr);
        return client;
    }
    
    void listen() {
        char in[512] = { 0 };
        char packetCode[18] = { 0 };
        char packetSerial[7] = { 0 };
        char packet[5136] = { 0 };
        auto client = waitForConnection();
        ok = true;
        JSEval("pane('sendPane')");
        JSEval("log('', '有人连接啦！', true)");
        
        while (running) {
            sleep(delay);
            if (!ok) { client = waitForConnection(); }
            auto serial = JSEval("document.getElementById('serial').value").ToInteger();
            auto code = (String) JSEval("document.getElementById('code').value").ToString();
            
            bzero(packetSerial, 7);
            bzero(packetCode, 18);
            memset(packet, ' ', sizeof(packet));
            strcpy(packet, "0000 ");
            packet[5] = 20;
            packet[4] = 0;

            memcpy(packetCode, code.utf8().data(), code.utf8().length() > 18 ? 18 : code.utf8().length());
            sprintf(packetSerial, "%ld", (long) serial);
            
            memcpy(&packet[34], packetCode, 18);
            memcpy(&packet[277], packetSerial, 6);
            ssize_t size = send(client, packet, sizeof(packet), 0);
            
            ok = (size == sizeof(packet));
            
            serial++;
            std::string cmd = std::string("document.getElementById('serial').value = ") + std::to_string(serial);
            // std::string log = std::string("log(") + std::to_string(serial) + ", '" + std::string(code.utf8().data()) + "', " + (ok ? "true" : "false") + ")";
            // cmd += "; " + log;
            JSEval(cmd.c_str());
            
            if (ok) {
                ssize_t received = recv(client, in, sizeof(in), 0);
                std::stringstream build;
                char hex[3] = { 0 };
                build << "0x";
                if (received <= 0) { ok = false; }
                for (int i = 0; i < sizeof(in) && ok; i++) {
                    bzero(hex, sizeof(hex));
                    sprintf(hex, "%2x", in[i]);
                    if (hex[0] == ' ') { hex[0] = '0'; }
                    build << hex;
                }
                std::string str = std::string("frames.push('") + build.str() + "')";
                JSEval(str.c_str());
                JSEval("frame(frames.length - 1)");
            }
        }
    }
    
private:
    RefPtr<Overlay> overlay;
    int sock = 0;
    bool ok = false;
    bool running = false;
    int delay = 1;
};

int main(int argc, const char * argv[]) {
    auto app = App::Create();
    auto win = Window::Create(app->main_monitor(), 500, 500, false, kWindowFlags_Titled);
    win->SetTitle("TireKit");
    app->set_window(win);
    MyApp myApp(win);
    
    app->Run();
    return 0;
}
