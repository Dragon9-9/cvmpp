# Publish CVM++ to GitHub

Follow these steps once to create a clean public repository.

## 1. Create the repository on GitHub

1. Go to [github.com/new](https://github.com/new)
2. Repository name: `cvmpp` (or `CVM++` — GitHub will normalize the URL)
3. Description: *C++17 scripting language with bytecode compiler and stack VM*
4. **Public**
5. Do **not** add README, .gitignore, or license (this repo already has them)
6. Click **Create repository**

## 2. Update README badges

This project is configured for **[@Dragon9-9](https://github.com/Dragon9-9)**.

## 3. Push from your machine

```bash
cd /path/to/cvmpp

git init
git add .
git commit -m "Initial public release: CVM++ interpreter, docs, and CI"

git branch -M main
git remote add origin git@github.com:Dragon9-9/cvmpp.git
git push -u origin main
```

Use HTTPS if you prefer:

```bash
git remote add origin https://github.com/Dragon9-9/cvmpp.git
```

## 4. Verify on GitHub

- **Actions** tab → workflow should pass (build + `make verify`)
- Open `docs/pdf/` PDFs in the browser (GitHub renders PDF previews)
- Pin the repo on your profile if it is a portfolio piece

## 5. Optional polish

- Add topics: `cpp`, `compiler`, `interpreter`, `virtual-machine`, `education`
- Link repo on resume/LinkedIn using the one-liner from [CV.md](CV.md)
- Regenerate docs after edits: `./scripts/build-docs.sh` then commit PDFs

## What gets committed

| Included | Excluded (`.gitignore`) |
|----------|-------------------------|
| Source, examples, docs PDFs | `build/cvmpp` binary |
| Scripts, CI workflow | `*.o`, CMake cache |
| LICENSE, README | Editor folders |
