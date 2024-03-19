/*_______________________________________________________________________________
    this space reserved for context and global classes
_______________________________________________________________________________ */
interface Context {
    connection: Connection
    pop_up: PopUp
}

type json = string;

function get_context(): Context {
    return (window as any).monopoly;
}

function create_unique_id(): string {
    return Math.random().toString(36).substr(2, 9);
}

class PopUp {
    public fire(title: string, message: string, type: 'error' | 'success', duration: number = 10000) {
        console.log(title, message, type, duration);
        this.create_popup(title, message, type, duration);
    }

    private create_popup (title: string, message: string, type: 'error' | 'success', duration: number) {
        const popup_container = document.getElementById('popup__container')

        if (!popup_container) {
            const container_element = `<div id="popup__container" class="popup__container"></div>`
            document.getElementsByTagName('body')[0].insertAdjacentHTML('beforeend', container_element)
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
        `

        document.getElementById('popup__container')?.insertAdjacentHTML('beforeend', popup)

        document.getElementById(`close-${unique_id}`)?.addEventListener('click', () => {
            const popup = document.getElementById(unique_id)
            popup!.remove()
        })

        setTimeout(() => {
            const popup = document.getElementById(unique_id)
            popup?.remove()
        }, duration)
    }
}

enum connection_actions {
    unknown = 1,
    need_authenticate,
    authenticate_failed,
    authenticate,
    authenticate_success,
    ping,
    pong,
    send_profile,
    transfer,
    transfer_success,
    transfer_received,
    transfer_no_funds,
    bad_request
}

interface NetworkingMessage {
    code: connection_actions
    [key: string]: any
}

abstract class Commands {
    abstract execute (data: NetworkingMessage): void;
}

class AuthenticateSuccess extends Commands {
    public execute (data: NetworkingMessage) {
        get_context().pop_up.fire(
            'Bem-vindo!',
            'Você foi autenticado com sucesso!',
            'success',
            3000
        )
    }
}

class Connection {
    private socket: WebSocket;
    private is_open: boolean;
    private messages: NetworkingMessage[] = []
    private messages_worker: any
    private commands: { [command: number]: Commands } = {}

    constructor() {
        this.socket = new WebSocket("ws://192.168.15.8:4444");
        this.is_open = false;

        this.socket.onopen = () => {
            this.is_open = true;
        }

        this.socket.onclose = () => {
            this.is_open = false;
        };

        this.socket.onerror = () => {
            this.is_open = false
            get_context().pop_up.fire('Conexão', 'Não foi possível conectar ao servidor', 'error', 5000)
        };

        this.socket.onmessage = (e) => {
            this.messages.push(JSON.parse(e.data))
        }

        this.messages_worker = setInterval(() => {
            if (this.messages.length) {
                const message = this.messages.shift()
                console.log(message)
            }
        }, 33)
    }

    private create_commands () {
        this.commands[connection_actions.authenticate_success] = new AuthenticateSuccess()
    }

    public isOpen (): boolean {
        return this.is_open;
    }

    public send (code: connection_actions, data: { [key: string]: any }): void {
        if (this.isOpen()) {
            this.socket.send(JSON.stringify({ code, ...data }))
        }
    }
}

if (!(window as any).monopoly) {
    (window as any).monopoly = {
        connection: new Connection(),
        pop_up: new PopUp()
    } as Context;
}

/*_______________________________________________________________________________
    this space reserved for login
_______________________________________________________________________________ */

document.getElementById('login_form')?.addEventListener('submit', (e) => {
    e.preventDefault()
    const formData = new FormData(e.target as HTMLFormElement)

    const data = {
        username: formData.get('username'),
        password: formData.get('password')
    }

    get_context().connection.send(connection_actions.authenticate, { ...data })
})
