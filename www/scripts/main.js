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
    execute(serverMessage) {
        var _a;
        if (!serverMessage.args.popup_disabled) {
            get_context().pop_up.fire('Monopoly Bank', `Bem vindo, ${serverMessage.args.username}!`, 'success', 3000);
        }
        sessionStorage.setItem('auth', JSON.stringify({
            username: serverMessage.args.username,
            password: serverMessage.args.password
        }));
        (_a = getLoginButton()) === null || _a === void 0 ? void 0 : _a.removeAttribute('disabled');
        if (window.location.pathname != '/bank') {
            window.location.href = '/bank';
        }
        else {
            if (this.callback)
                this.callback();
        }
    }
    setCallback(callback) {
        this.callback = callback;
    }
}
class AuthenticateFailed extends Commands {
    execute(serverMessage) {
        var _a;
        get_context().pop_up.fire('Monopoly Bank', 'Usuário ou senha incorretos', 'error', 5000);
        sessionStorage.removeItem('auth');
        (_a = getLoginButton()) === null || _a === void 0 ? void 0 : _a.removeAttribute('disabled');
    }
}
class ProfileCommand extends Commands {
    execute(serverMessage) {
        console.log(serverMessage);
    }
}
class Connection {
    constructor() {
        this.socket = null;
        this.is_open = false;
        this.messages = [];
        this.commands = {};
        this.args_repository = new Map();
        this.createSocket();
        this.createCommands();
        this.createWorker();
    }
    createSocket() {
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
            const msg = JSON.parse(e.data);
            if (msg.id && this.args_repository.has(msg.id)) {
                const args = this.args_repository.get(msg.id);
                this.args_repository.delete(msg.id);
                if (args) {
                    msg.args = args;
                }
            }
            this.messages.push(msg);
            if (!this.messages_worker) {
                this.createWorker();
            }
        };
    }
    openSocket() {
        var _a;
        if (this.is_open) {
            (_a = this.socket) === null || _a === void 0 ? void 0 : _a.close();
        }
        this.createSocket();
    }
    createWorker() {
        this.messages_worker = setInterval(() => {
            while (this.messages.length) {
                const message = this.messages.shift();
                if (message) {
                    const command = this.commands[`${message.code}`];
                    command.execute(message);
                }
            }
            if (!this.messages.length) {
                clearInterval(this.messages_worker);
                this.messages_worker = null;
            }
        }, 10);
    }
    createCommands() {
        this.commands[connection_actions.authenticate_success] = new AuthenticateSuccess();
        this.commands[connection_actions.authenticate_failed] = new AuthenticateFailed();
        this.commands[connection_actions.send_profile] = new ProfileCommand();
    }
    isOpen() {
        return this.is_open;
    }
    send(data) {
        var _a;
        if (this.isOpen()) {
            const unique_id = create_unique_id();
            if (data.args) {
                this.args_repository.set(unique_id, data.args);
            }
            (_a = this.socket) === null || _a === void 0 ? void 0 : _a.send(JSON.stringify(Object.assign(Object.assign({}, data), { id: unique_id })));
        }
        else {
            console.error('Connection is not open');
        }
    }
}
window.monopoly = {
    connection: new Connection(),
    pop_up: new PopUp()
};
// reconnect connection on change page
if (window.location.pathname === '/bank') {
    if (!sessionStorage.getItem('auth')) {
        window.location.href = '/';
    }
    else {
        const context = get_context();
        if (!context.connection.isOpen()) {
            context.connection.openSocket();
        }
        const wait_connection = setInterval(() => {
            if (context.connection.isOpen()) {
                clearInterval(wait_connection);
                let user_raw = sessionStorage.getItem('auth');
                if (user_raw) {
                    const data = JSON.parse(user_raw);
                    const { username, password } = data;
                    context.connection.commands[connection_actions.authenticate_success].setCallback(() => {
                        context.connection.send({
                            code: connection_actions.send_profile
                        });
                    });
                    context.connection.send({
                        code: connection_actions.authenticate,
                        args: { username, password, popup_disabled: true },
                        username,
                        password
                    });
                }
            }
        }, 100);
    }
}
/*_______________________________________________________________________________
    this space reserved for login
_______________________________________________________________________________ */
function getLoginButton() {
    return document.getElementById('login-dispatch');
}
(_a = document.getElementById('login_form')) === null || _a === void 0 ? void 0 : _a.addEventListener('submit', (e) => {
    e.preventDefault();
    const formData = new FormData(e.target);
    const data = {
        username: formData.get('username'),
        password: formData.get('password')
    };
    get_context().connection.send(Object.assign({ code: connection_actions.authenticate, args: Object.assign({}, data) }, data));
    const button = document.getElementById('login-dispatch');
    if (button)
        button.setAttribute('disabled', 'true');
});
