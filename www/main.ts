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
    code: connection_actions,
    args?: { [key:string]: any }
    [key: string]: any
}

abstract class Commands {
    abstract execute (serverMessage: NetworkingMessage): void;
}

class AuthenticateSuccess extends Commands {
    private callback: Function | undefined

    public execute (serverMessage: NetworkingMessage) {
        if (!serverMessage.args!.popup_disabled) {
            get_context().pop_up.fire(
                'Monopoly Bank',
                `Bem vindo, ${serverMessage.args!.username}!`,
                'success',
                3000,
            )
        }

        sessionStorage.setItem('auth', JSON.stringify({
            username: serverMessage.args!.username,
            password: serverMessage.args!.password
        }))

        getLoginButton()?.removeAttribute('disabled')

        if (window.location.pathname != '/bank') {
            window.location.href = '/bank'
        } else {
            if (this.callback) this.callback()
        }
    }

    public setCallback (callback: Function) {
        this.callback = callback
    }
}

class AuthenticateFailed extends Commands {
    public execute (serverMessage: NetworkingMessage) {
        get_context().pop_up.fire(
            'Monopoly Bank',
            'Usuário ou senha incorretos',
            'error',
            5000,
        )

        sessionStorage.removeItem('auth')
        getLoginButton()?.removeAttribute('disabled')
    }
}

class ProfileCommand extends Commands {
    public execute (serverMessage: NetworkingMessage) {
        console.log(serverMessage)
    }
}

class Connection {
    private socket: WebSocket | null = null;
    private is_open: boolean = false;
    private messages: NetworkingMessage[] = []
    private messages_worker: any
    public commands: { [command: number]: Commands } = {}
    private args_repository: Map<string, { [key: string]: any }> = new Map()

    constructor() {
        this.createSocket()
        this.createCommands()
        this.createWorker()
    }

    private createSocket (): void {
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
            const msg = JSON.parse(e.data) as NetworkingMessage

            if (msg.id && this.args_repository.has(msg.id)) {
                const args = this.args_repository.get(msg.id)
                this.args_repository.delete(msg.id)

                if (args) {
                    msg.args = args
                }
            }

            this.messages.push(msg)

            if (!this.messages_worker) {
                this.createWorker()
            }
        }
    }

    public openSocket (): void {
        if (this.is_open) {
            this.socket?.close()
        }

        this.createSocket()
    }

    private createWorker (): void {
        this.messages_worker = setInterval(() => {
            while (this.messages.length) {
                const message = this.messages.shift()

                if (message) {
                    const command = this.commands[`${message.code}`]
                    command.execute(message)
                }
            }

            if (!this.messages.length) {
                clearInterval(this.messages_worker)
                this.messages_worker = null
            }
        }, 10)
    }

    private createCommands () {
        this.commands[connection_actions.authenticate_success] = new AuthenticateSuccess()
        this.commands[connection_actions.authenticate_failed] = new AuthenticateFailed()
        this.commands[connection_actions.send_profile] = new ProfileCommand()
    }

    public isOpen (): boolean {
        return this.is_open;
    }

    public send (data: NetworkingMessage): void {

        if (this.isOpen()) {
            const unique_id = create_unique_id()

            if (data.args) {
                this.args_repository.set(unique_id, data.args)
            }

            this.socket?.send(JSON.stringify({ ...data, id: unique_id }))
        } else {
            console.error('Connection is not open')
        }
    }
}

(window as any).monopoly = {
    connection: new Connection(),
    pop_up: new PopUp()
} as Context;

// reconnect connection on change page
if (window.location.pathname === '/bank') {

    if (!sessionStorage.getItem('auth')) {
        window.location.href = '/'
    } else {
        const context = get_context()
        if (!context.connection.isOpen()) {
            context.connection.openSocket()
        }

        const wait_connection = setInterval(() => {
            if (context.connection.isOpen()) {
                clearInterval(wait_connection)

                let user_raw = sessionStorage.getItem('auth')

                if (user_raw) {
                    const data: { [key: string]: any } = JSON.parse(user_raw)
                    const { username, password } = data;

                    (context.connection.commands[connection_actions.authenticate_success] as AuthenticateSuccess).setCallback(() => {
                        context.connection.send({
                            code: connection_actions.send_profile
                        })
                    })

                    context.connection.send({
                        code: connection_actions.authenticate,
                        args: { username, password, popup_disabled: true },
                        username,
                        password
                    })
                }
            }
        }, 100)
    }
}

/*_______________________________________________________________________________
    this space reserved for login
_______________________________________________________________________________ */

function getLoginButton() {
    return document.getElementById('login-dispatch')
}

document.getElementById('login_form')?.addEventListener('submit', (e) => {
    e.preventDefault()
    const formData = new FormData(e.target as HTMLFormElement)

    const data = {
        username: formData.get('username'),
        password: formData.get('password')
    }

    get_context().connection.send({
        code: connection_actions.authenticate,
        args: { ...data },
        ...data,
    })

    const button = document.getElementById('login-dispatch')

    if (button)
        button.setAttribute('disabled', 'true')
})
