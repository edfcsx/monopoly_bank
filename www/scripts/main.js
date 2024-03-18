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
class Connection {
    constructor() {
        this.socket = new WebSocket("ws://192.168.15.8:4444");
        this.is_open = false;
        this.socket.onopen = () => {
            this.is_open = true;
            get_context().pop_up.fire('Conexão!', 'Você recebeu uma transferência do jogador Mortadela, lorem ipsum dev alone kaiju', 'error');
        };
        this.socket.onclose = () => {
            this.is_open = false;
        };
        this.socket.onerror = () => this.is_open = false;
    }
    isOpen() {
        return this.is_open;
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
    console.log('HERE:>', data, get_context().connection.isOpen());
});
