[![ru](https://img.shields.io/badge/lang-ru-green.svg)](README.ru-RU.md)

Confirm Email
-
**ConfirmEmail** is a module for [Apostol](https://github.com/apostoldevel/apostol).

Description
-
**ConfirmEmail** handles email address verification via a one-click confirmation link. It intercepts `GET` requests to the `/confirm/email/<code>` path, validates the verification code through the API, and redirects the user to a configured success or error page.

Works with the `verification` module from [db-platform](https://github.com/apostoldevel/db-platform).

How it works
-
1. A user clicks a verification link, e.g. `https://example.com/confirm/email/abc123`.
2. The module extracts the verification code from the URL path.
3. It authenticates against the API using the OAuth2 `client_credentials` grant.
4. It calls `POST /api/v1/verification/email/confirm` with the extracted code.
5. On success — redirects to the configured `uri`.
6. On failure — redirects to the configured `error_uri`.

URL format
-
```
GET /confirm/email/<code>
```

Where `<code>` is the verification code sent to the user's email address.

Operating modes
-
| Mode | Behavior |
|------|----------|
| `native` | Server-side: the module calls the API directly and redirects the user. |
| `site` | Serves a static `index.html` from the URL path; the browser-side JavaScript handles the confirmation. |

Configuration
-
The module uses a separate config file specified in the Apostol configuration:

```ini
[module/ConfirmEmail]
enable=true
config=conf/confirm_email.conf
```

`conf/confirm_email.conf` sections:

| Section | Key | Default | Description |
|---------|-----|---------|-------------|
| `[main]` | `mode` | `site` | Operating mode: `native` or `site` |
| `[redirect]` | `uri` | `/api/v1/verification/email` | Redirect URL on successful confirmation |
| `[redirect]` | `error_uri` | same as `uri` | Redirect URL on failure |

Installation
-
Follow the build and installation instructions for [Apostol](https://github.com/apostoldevel/apostol#build-and-installation).
