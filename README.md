# Holdfast

A tower-defense game in C++20 and [raylib](https://www.raylib.com/).

**You are helping test something.** We're deciding whether next semester's CS 1410 can build a game like this one. Before we commit to it, we need to know it runs on *real student laptops* — not just the instructor's. Yours is one of those laptops. Thanks.

It should take about 20 minutes, most of it waiting. **Nothing here installs anything on your computer** except Docker; everything else happens inside a container that you can delete afterward.

---

## What you need

- **Docker Desktop** (Windows/Mac) or **Docker Engine** (Linux), running
- **VS Code** with the **Dev Containers** extension
- About **5 GB of free disk** and a decent internet connection for the first build

---

## Do this

### 1. Clone and open

```bash
git clone https://github.com/Hadzik-CS1410/holdfast-prototype.git
cd holdfast-prototype
code .
```

VS Code will show a popup in the corner: **"Reopen in Container."** Click it.

> No popup? Press `F1`, type **Dev Containers: Reopen in Container**, press Enter.

**The first build takes several minutes** — it downloads a Linux image with a compiler, graphics libraries, and a small desktop. Later ones are instant. Let it finish; the bottom-left corner says *Dev Container: CS 1410 Game Dev* when it's done.

### 2. Open the game desktop in your browser

Go to **http://localhost:6080** and click **Connect**. There's no password.

You'll get an empty gray desktop. That's correct — it's a tiny Linux desktop running inside the container, and it's where the game window will appear.

> VS Code's **Ports** panel lists it as *Game desktop* if you'd rather click than type.
>
> In the noVNC side panel (the little arrow on the left edge), **Settings → Scaling Mode → Local Scaling** makes the desktop fit your browser window.

### 3. Run the smoke test

In the VS Code terminal (``Ctrl+` ``), inside the container:

```bash
./smoketest.sh
```

It builds the game, runs the tests, and lets a scripted bot play all six waves. **The first build takes a few minutes.** When it's done it prints a report.

### 4. Play it yourself

```bash
./build/holdfast
```

Switch to your **localhost:6080** browser tab — the game window is there.

| Input | Action |
|---|---|
| `1` / `2` / `3` or click a HUD button | choose Arrow / Cannon / Frost |
| left-click grass | build the selected tower |
| right-click a tower | sell it (70% refund) |
| `SPACE` | send the next wave |
| `F` double speed · `P` pause · `R` restart · `ESC` cancel |

Play for a minute. **Pay attention to how the mouse and keyboard feel** — that's the part we most need your opinion on. If it feels laggy, say so. That's a real finding, not a complaint.

Close the window when you're done, or press `Ctrl+C` in the terminal.

### 5. Tell us what happened

Open an issue on this repo: **Issues → New issue → "Smoke test report."** Paste the report `smoketest.sh` printed, fill in the two or three blanks, and submit.

**Report it even if everything worked.** "Worked fine on an M2 Mac" is exactly as useful to us as a failure — maybe more.

---

## If something goes wrong

Tell us in an issue. That's the point of the exercise — **a failure here is a successful test.** Include what you saw. A few known things:

| Problem | Try this |
|---|---|
| "Reopen in Container" never appears | Install the **Dev Containers** extension, then `F1` → *Dev Containers: Reopen in Container* |
| Container build fails partway | Make sure Docker Desktop is actually running, then `F1` → *Dev Containers: Rebuild Container* |
| localhost:6080 won't connect | Give it ~30 seconds after the container starts. Check the **Ports** panel shows 6080 |
| The game window never appears | Make sure you ran it *inside* the container terminal, not your own |
| Black or torn window | Say so in the issue, with your OS and chip — this is a thing we want to know |

---

## How to play (if you want to actually win)

- **Arrow**: cheap, fast, single target. Weak against armored Brutes.
- **Cannon**: slow, splash damage. Good against crowds and Brutes.
- **Frost**: barely hurts, slows the lead enemy it hasn't already slowed.
- **Grunt** · **Brute** (armor 5) · **Splitter** (splits into two fast Splitlings when it dies)

Towers only go on grass, and enemies take a life off the counter at the blue square. Lose 20 lives and it's over.

---

## For the curious: how it's laid out

```
include/, src/core/   the game's rules -- no raylib, fully unit tested
src/game/             main loop, input, render.cpp -- the only files that touch raylib
levels/               meadow.txt (the map) and meadow.waves (the waves) -- edit to rebalance
tests/test_core.cpp   GoogleTest
```

The rules of the game don't know graphics exist. That's why `./build/run_tests` works with no window open, and it's the idea the whole course would be built around.

Want to break something? `levels/meadow.waves` is plain text. Make wave 1 enormous and watch what happens.
