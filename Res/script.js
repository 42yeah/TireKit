function header(newHeader) {
    const headers = document.getElementsByName('header');
    for (let i = 0; i < headers.length; i++) {
        headers[i].innerHTML = newHeader;
    }
}

function pane(which) {
    let panes = [
                 'errPane',
                 'sendPane',
                 'recvPane'
                 ];
    
    let buttons = [
                   'sendButton',
                   'recvButton'
                   ];
    for (let i = 0; i < panes.length; i++) {
        document.getElementById(panes[i]).classList.add('hide');
    }
    for (let i = 0; i < buttons.length; i++) {
        document.getElementById(buttons[i]).classList.remove('active');
    }
    if (which == 'sendPane') { document.getElementById('sendButton').classList.add('active'); }
    if (which == 'recvPane') { document.getElementById('recvButton').classList.add('active'); }
    
    document.getElementById(which).classList.remove('hide');
}

let total = 0;
function log(serial, content, ok) {
    let body = document.getElementById('body');
    body.innerHTML += `
    <tr>
    <td>` + new Date().getTime() + `</td>
    <td>` + serial + `</td>
    <td>` + content + `</td>
    <td>` + ok + `</td>
    </tr>
    `;
    total++;
    document.getElementById('total').innerHTML = '一共 ' + total + ' 条';
}

let frames = [];
let currentFrame = 0;
function frame(which) {
    if (frames.length == 0 || which < 0) {
        if (frame.length == 0) {
            nextFrame.classList.add('disabled');
            prevFrame.classList.add('disabled');
        }
        document.getElementById('frame').value = '0x0';
        return;
    }
    currentFrame = which;
    let content = frames[currentFrame];
    document.getElementById('frame').value = content;
    let nextFrame = document.getElementById('nextFrame');
    let prevFrame = document.getElementById('prevFrame');
    nextFrame.classList.remove('disabled');
    prevFrame.classList.remove('disabled');
    if (currentFrame == 0) { prevFrame.classList.add('disabled'); }
    if (currentFrame == frames.length - 1) { nextFrame.classList.add('disabled'); }
}

