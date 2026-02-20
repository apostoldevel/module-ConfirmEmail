[![en](https://img.shields.io/badge/lang-en-green.svg)](README.md)

Подтверждение адреса электронной почты
-
**ConfirmEmail** — модуль для [Апостол](https://github.com/apostoldevel/apostol).

Описание
-
**ConfirmEmail** обрабатывает подтверждение адреса электронной почты по ссылке из письма. Перехватывает `GET`-запросы по пути `/confirm/email/<code>`, проверяет код через API и перенаправляет пользователя на страницу успеха или ошибки.

Работает с модулем `verification` платформы [db-platform](https://github.com/apostoldevel/db-platform).

Принцип работы
-
1. Пользователь переходит по ссылке подтверждения, например: `https://example.com/confirm/email/abc123`.
2. Модуль извлекает код подтверждения из пути URL.
3. Выполняет аутентификацию через API по OAuth2 (`client_credentials`).
4. Вызывает `POST /api/v1/verification/email/confirm` с извлечённым кодом.
5. При успехе — перенаправляет на настроенный `uri`.
6. При ошибке — перенаправляет на `error_uri`.

Формат URL
-
```
GET /confirm/email/<code>
```

Где `<code>` — код подтверждения, отправленный на адрес электронной почты пользователя.

Режимы работы
-
| Режим | Поведение |
|-------|----------|
| `native` | Серверная обработка: модуль вызывает API напрямую и перенаправляет пользователя. |
| `site` | Отдаёт статический `index.html` из пути URL; JavaScript на стороне браузера выполняет подтверждение. |

Настройка
-
Модуль использует отдельный конфигурационный файл, указанный в конфигурации Апостол:

```ini
[module/ConfirmEmail]
enable=true
config=conf/confirm_email.conf
```

Секции `conf/confirm_email.conf`:

| Секция | Ключ | По умолчанию | Описание |
|--------|------|-------------|----------|
| `[main]` | `mode` | `site` | Режим работы: `native` или `site` |
| `[redirect]` | `uri` | `/api/v1/verification/email` | URL перенаправления при успехе |
| `[redirect]` | `error_uri` | то же, что `uri` | URL перенаправления при ошибке |

Установка
-
Следуйте указаниям по сборке и установке [Апостол](https://github.com/apostoldevel/apostol#%D1%81%D0%B1%D0%BE%D1%80%D0%BA%D0%B0-%D0%B8-%D1%83%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0).
