"use strict";
var _a;
function get_context() {
    return window.monopoly;
}
function create_unique_id() {
    return Math.random().toString(36).substr(2, 9);
}
class PopUp {
    fire(title, message, type, duration = 10000) {
        console.log(title, message, type, duration);
        this.create_popup(title, message, type, duration);
    }
    create_popup(title, message, type, duration) {
        var _a, _b;
        const popup_container = document.getElementById('popup__container');
        if (!popup_container) {
            const container_element = `<div id="popup__container" class="popup__container"></div>`;
            document.getElementsByTagName('body')[0].insertAdjacentHTML('beforeend', container_element);
        }
        const unique_id = create_unique_id();
        const popup = `
            <div id="${unique_id}" class="popup popup-${type}">
                <div class="popup__header">
                    <span class="popup__header__title">${title}</span>
                    <span id="close-${unique_id}" class="popup__header__close">X</span>
                </div>
                
                <div class="popup__body">
                    <span>${message}</span>
                </div>
            </div>
        `;
        (_a = document.getElementById('popup__container')) === null || _a === void 0 ? void 0 : _a.insertAdjacentHTML('beforeend', popup);
        (_b = document.getElementById(`close-${unique_id}`)) === null || _b === void 0 ? void 0 : _b.addEventListener('click', () => {
            const popup = document.getElementById(unique_id);
            popup.remove();
        });
        setTimeout(() => {
            const popup = document.getElementById(unique_id);
            popup === null || popup === void 0 ? void 0 : popup.remove();
        }, duration);
    }
}
var connection_actions;
(function (connection_actions) {
    connection_actions[connection_actions["unknown"] = 1] = "unknown";
    connection_actions[connection_actions["need_authenticate"] = 2] = "need_authenticate";
    connection_actions[connection_actions["authenticate_failed"] = 3] = "authenticate_failed";
    connection_actions[connection_actions["authenticate"] = 4] = "authenticate";
    connection_actions[connection_actions["authenticate_success"] = 5] = "authenticate_success";
    connection_actions[connection_actions["ping"] = 6] = "ping";
    connection_actions[connection_actions["pong"] = 7] = "pong";
    connection_actions[connection_actions["send_profile"] = 8] = "send_profile";
    connection_actions[connection_actions["transfer"] = 9] = "transfer";
    connection_actions[connection_actions["transfer_success"] = 10] = "transfer_success";
    connection_actions[connection_actions["transfer_received"] = 11] = "transfer_received";
    connection_actions[connection_actions["transfer_no_funds"] = 12] = "transfer_no_funds";
    connection_actions[connection_actions["bad_request"] = 13] = "bad_request";
})(connection_actions || (connection_actions = {}));
class Commands {
}
class AuthenticateSuccess extends Commands {
    execute(data) {
        get_context().pop_up.fire('Bem-vindo!', 'Você foi autenticado com sucesso!', 'success', 3000);
    }
}
class Connection {
    constructor() {
        this.messages = [];
        this.commands = {};
        this.socket = new WebSocket("ws://192.168.15.8:4444");
        this.is_open = false;
        this.socket.onopen = () => {
            this.is_open = true;
        };
        this.socket.onclose = () => {
            this.is_open = false;
        };
        this.socket.onerror = () => {
            this.is_open = false;
            get_context().pop_up.fire('Conexão', 'Não foi possível conectar ao servidor', 'error', 5000);
        };
        this.socket.onmessage = (e) => {
            this.messages.push(JSON.parse(e.data));
        };
        this.messages_worker = setInterval(() => {
            if (this.messages.length) {
                const message = this.messages.shift();
                console.log(message);
            }
        }, 33);
    }
    create_commands() {
        this.commands[connection_actions.authenticate_success] = new AuthenticateSuccess();
    }
    isOpen() {
        return this.is_open;
    }
    send(code, data) {
        if (this.isOpen()) {
            this.socket.send(JSON.stringify(Object.assign({ code }, data)));
        }
    }
}
if (!window.monopoly) {
    window.monopoly = {
        connection: new Connection(),
        pop_up: new PopUp()
    };
}
/*_______________________________________________________________________________
    this space reserved for login
_______________________________________________________________________________ */
(_a = document.getElementById('login_form')) === null || _a === void 0 ? void 0 : _a.addEventListener('submit', (e) => {
    e.preventDefault();
    const formData = new FormData(e.target);
    const data = {
        username: formData.get('username'),
        password: formData.get('password')
    };
    get_context().connection.send(connection_actions.authenticate, Object.assign({}, data));
});
