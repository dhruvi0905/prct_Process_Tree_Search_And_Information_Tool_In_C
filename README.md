# Process Tree Explorer - A Linux Process Management Tool in C

## About

🔍 Process Tree Explorer is a lightweight C utility that manages and explores Linux processes via the `/proc` filesystem. It maps parent-child relationships, identifies zombie processes, sends signals to process trees, and helps administrators debug complex hierarchies efficiently.

## Features

- Display direct children, grandchildren, and non-direct descendants
- Identify and list zombie (defunct) and orphan processes
- Kill parent processes of zombies
- Send signals (SIGKILL, SIGSTOP, SIGCONT) to entire process trees
- List siblings and zombie siblings
- Supports counting defunct descendants
- Robust error handling and input validation

## Requirements

- GCC compiler
- Linux-based operating system with `/proc` filesystem

## How to Compile

```bash
gcc -o prct prct_dhruvi_dobariya_110159758.c
```

## How to Run

```bash
./prct [root_pid] [target_pid] [option]
```

## Available Options

| Option | Description                       |
| ------ | --------------------------------- |
| -id    | List direct children              |
| -gc    | List grandchildren                |
| -ds    | List non-direct descendants       |
| -df    | List defunct (zombie) descendants |
| -dc    | Count defunct descendants         |
| -op    | List orphan descendants           |
| -do    | Check if process is defunct       |
| -so    | Check if process is orphan        |
| -lg    | List siblings                     |
| -lz    | List zombie siblings              |
| --pz   | Kill parents of zombie processes  |
| -sk    | Send SIGKILL to all descendants   |
| -st    | Send SIGSTOP to all descendants   |
| -dt    | Send SIGCONT to all descendants   |
| -rp    | Kill a specific process           |

> If no option is provided, it prints the PID and PPID of the target process.

## Example

```bash
./prct 1 2345 -id   # Lists direct children of process 2345
./prct 1 2345 -df   # Lists all defunct descendants of process 2345
./prct 1 2345 -sk   # Sends SIGKILL to all descendants of process 2345
```

## Notes

- The tool must be run with sufficient privileges to manage and signal processes.
- Use destructive options (like `-sk`, `-rp`) cautiously.

## License

Open-source project for educational and system administration use.

---

*Developed by Dhruvi Dobariya*

