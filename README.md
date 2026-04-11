_This project has been created as part of the 42 curriculum by ahirzall, drahwanj, aeleimat._

## Description

**ft_irc** is an implementation of a minimal **Internet Relay Chat (IRC) server** in **C++98**, developed as a 42 School project. The program listens on a TCP port, accepts multiple concurrent clients, and exchanges messages according to common IRC client expectations so that a **reference IRC client** (for example **HexChat**) can connect, register, join channels, and use the required operator features.

The goal is to understand **event-driven network programming**: non-blocking sockets, a single **I/O multiplexing** call for all file descriptors, and **line-based protocol parsing** (including **fragmented reads**), without forking and without linking server-to-server.

This repository contains **only the server**; it does not implement an IRC client and does not implement **server-to-server** links, in line with the subject.

## Instructions

### Prerequisites

- A **Unix-like** environment (Linux, or **WSL** on Windows).
- A **C++** compiler (**not** plain C) capable of building code with the **C++98** standard (for example **c++** with `-std=c++98`).
- Standard development tools: `make`.

The project uses **POSIX** sockets (`socket`, `bind`, `listen`, `accept`, `recv`, `send`, `fcntl`, `poll`, etc.). It is **not** built as a native Windows executable.

### Installation

```bash
git clone git@vogsphere.42amman.com:vogsphere/intra-uuid-ac50f5a6-0845-4bbc-90b0-4568942cfaaa-7291309-ahirzall ircserv
```

### Compilation

From the repository root:

```bash
make
```

This produces the executable **`ircserv`**, compiled with **`-Wall -Wextra -Werror -std=c++98`** as required.

Other Makefile targets:

| Target         | Action                       |
| -------------- | ---------------------------- |
| `make` / `all` | Build `ircserv`              |
| `make clean`   | Remove object files          |
| `make fclean`  | Remove objects and `ircserv` |
| `make re`      | `fclean` then `all`          |

### Execution

```bash
./ircserv <port> <password>
```

| Argument     | Meaning                                                                                                                                                      |
| ------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `<port>`     | TCP port on which the server listens (this build validates a numeric port in the **1024–65535** range; privileged ports require appropriate OS permissions). |
| `<password>` | Connection password. Clients must authenticate with **`PASS`** using this value before registration completes.                                               |

Example:

```bash
./ircserv 6667 my_server_password
```

Stop the server with **Ctrl+C** (`SIGINT` / `SIGQUIT` are handled for graceful shutdown).

### Quick manual test

The subject recommends verifying **partial input** (e.g. with **netcat**). On many Linux distributions, **`nc -C`** sends **CRLF** line endings, which IRC lines expect:

```bash
nc -C 127.0.0.1 6667
```

Then register (order may vary before completion, but **PASS**, **NICK**, and **USER** must all be accepted):

```text
PASS my_server_password
NICK tester
USER tester 0 * :Test User
```

For multi-user and operator scenarios, open **one terminal for the server** and **one terminal per client**.

### Optional checks

- **Valgrind** (memory errors): `valgrind --leak-check=full ./ircserv <port> <password>`
- Shell scripts in the repository root (if present), e.g. `irc_tester.sh`, are intended to be run from a Unix shell after the server is listening.

## Features overview

Commands implemented in the server include, among others: **PASS**, **NICK**, **USER**, **QUIT**, **JOIN**, **PART**, **PRIVMSG**, **MODE**, **TOPIC**, **KICK**, **INVITE**, and **WHO**. Exact syntax and numeric replies follow the project’s **`Replies.hpp`** and handler logic under `src/`.

## Resources

Classic references for IRC and network programming:

- [RFC 1459](https://www.rfc-editor.org/rfc/rfc1459) — _Internet Relay Chat Protocol_ (historical baseline).
- [RFC 2812](https://www.rfc-editor.org/rfc/rfc2812) — _Internet Relay Chat: Client Protocol_ (later client command documentation; useful for cross-checking behavior).
- [RFC 2813](https://www.rfc-editor.org/rfc/rfc2813) — _Internet Relay Chat: Server Protocol_ (background only; this project does **not** implement server linking).
- `man 2 poll`, `man 2 socket`, `man 2 send`, `man 2 recv`, `man 2 fcntl` — POSIX manual pages on your system.
- [Beej’s Guide to Network Programming](https://beej.us/guide/bgnet/) — practical introduction to TCP/IP sockets on Unix-like systems.

Official project specification:

- **`en.subject.pdf`** (42 **ft_irc** / **Internet Relay Chat** subject), version distributed with this repository.

### Artificial intelligence

We used AI tools as **research and documentation helpers**, not as a substitute for reading the subject or understanding our own code.

| Area                                    | How AI helped                                                                                                                                                                                                                                                                                                                   |
| --------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Gathering resources and information** | Finding and sorting useful material (RFCs, socket/`poll` guides, IRC client behaviour) and suggesting **clear examples** of protocol flows—e.g. registration order, what a numeric reply means, or how a reference client typically talks to a server. We still **verified** everything against `en.subject.pdf` and our tests. |
| **Markdown and this README**            | Improving **structure, wording, and Markdown formatting** (headings, lists, tables) so the README stays readable and matches what 42 asks for.                                                                                                                                                                                  |
| **Commands, edge cases, and examples**  | Drafting **shell examples** (build, run, `nc -C`, Valgrind), **multi-client** test ideas, and reminders about **edge cases** (partial lines, CRLF, wrong `PASS`, channel modes). We ran the commands ourselves and adjusted when something did not match our environment.                                                       |

The implementation in `src/` was written and debugged by the team; where AI suggested a fact or a command sequence, we **checked it** before relying on it.

---

_This README is written in English, as required by the subject._
