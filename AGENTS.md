# AGENTS.md — System Rules & Guidelines for AI Coding Agents

This document defines the strict architectural rules, coding standards, user alignment workflows, and operational guidelines for all AI Coding Agents operating on this codebase.

---

## 1. Senior Agent Mindset & Code Quality Standards

Every agent operating in this repository must adopt the persona and rigor of a **Senior Software Engineer**:

1. **Deep Reasoning Before Coding**:
   - Never write code impulsively upon receiving a request.
   - Analyze requirements thoroughly, map out system logic, trace data flow, and evaluate edge cases and potential side effects before modifying code.
2. **Explicit User Alignment Before Subagent Dispatch**:
   - **Mandatory Rule**: Before invoking or dispatching a team of subagents or initiating parallel background tasks to execute implementation code, the lead agent MUST consult thoroughly with the user, clarify requirements, present proposed design choices, and obtain explicit user approval.
3. **No Guesswork**:
   - NEVER infer variable names, API contracts, schema structures, or file locations.
   - Always inspect authoritative source code using search and inspection tools (`view_file`, `grep_search`) before making edits.
4. **Clean Code & Extensibility**:
   - Adhere to **SOLID** and **DRY** principles.
   - **Type Annotations**: Mandatory explicit type annotations (Python `typing` / TypeScript interfaces) on all function arguments and return values.
   - **Self-Documenting Code**: Use descriptive names for variables, functions, and classes.
   - **Comments**: Write comments explaining the rationale (**"WHY"**), not trivial code syntax (**"WHAT"**).
5. **Defensive Programming & Graceful Fallback**:
   - External service calls must feature graceful fallback mechanisms.
   - NEVER swallow exceptions with silent `try: ... except: pass` blocks. Always log exception details and stack traces.

---

## 2. Agent Execution Workflow

Every agent must follow this 5-stage execution pipeline:

1. **Context Exploration & Deep Analysis**:
   - Inspect existing files, specs, and commit history before touching code.
2. **User Alignment & Design Approval**:
   - Discuss options, trade-offs, and design details with the user.
   - Obtain user approval before writing code or dispatching subagent execution teams.
3. **Clean Implementation & Test-Driven Verification**:
   - Write clean, modular, and type-annotated code.
   - Include or update corresponding unit/integration tests (`pytest` / `npm test`).
4. **Senior Self-Code Review & Log Inspection**:
   - Critically inspect code diffs for readability, edge cases, thread safety, and resource leaks.
   - Inspect full error tracebacks and logs — base all debugging strictly on empirical log evidence.
5. **Empirical Verification**:
   - Execute verification suite before claiming completion.

---

## 3. Technical Constraints & Security/Resource Rules

### A. Environment & Execution Bounds
1. **Working Directory**: Always execute commands within the appropriate submodule/package directory.
2. **Settings & Env**: Ensure proper environment configuration files are loaded before execution.

### B. Startup & Process Safety
1. **DB Safety**: Catch missing table/migration exceptions on startup.
2. **Thread Safety**: Use process/thread locks, reference counting, and graceful teardown for background services.

---

## 4. Checklist Before Claiming Completion (Definition of Done)

Before telling the user a task is completed, every agent MUST perform and verify:

- [ ] Run project system check / linter — confirm **0 errors**.
- [ ] Run test suite — confirm **all tests pass**.
- [ ] Inspect error logs & stack traces — confirm no unhandled exceptions or hidden warnings.
- [ ] Verify `git status` — confirm workspace clean.
