# Agent Instructions

## Language

- Communicate with the user in Japanese in chat, including progress updates and final responses.
- Write documentation, code, code comments, issues, and pull requests in English.
- Write commit messages in English.
- Preserve existing identifiers, product names, and quoted source material when appropriate.

## PCB Development

- Use the EasyEDA MCP / `easyeda-api` skill for schematic and PCB work.
- Follow [the PCB development workflow](docs/pcb-development.md).
- Read the installed skill and the relevant API signatures before editing through EasyEDA.
- Prioritize the user's latest instructions and mechanical layout over preliminary concept documents.
- Preserve user-placed components, test pads, logos, and keycap outlines unless a change is requested.
- Keep changes small and reversible. Verify the resulting design and update its versioned design record.
- Distinguish design checks from physical prototype validation; do not claim unperformed tests passed.
