Your time is valuable, so is mine. We'll have to follow a few rules to make sure we get the most out of our combined efforts. I promise to keep the ruleset as small and simple as possible.

# Introduction
I do have some ideas where I want to take this project, but I'm open to your suggestions. We can talk about improvements, new features etc. - just open an issue and tell me about it. However, being the originator, I do reserve the right to have the final word if we cannot agree on something.

# Contributing code
This is where I get and need to be a little picky so things don't spiral out of control. If you have questions about this, please ask them before you're getting involved - especially if you're thinking about (big) new features or massive changes.

## Tools
I use [Visual Studio Community 2015](https://www.microsoft.com/en-us/download/details.aspx?id=48146) and [Qt 5.6.1](http://download.qt.io/official_releases/qt/5.6/5.6.1/), I suggest you do the same. Install Visual Studio first, then Qt. Qt Creator will automatically detect MSVC2015, no additional configuration required. Works pretty much out-of-the-box :+1:.

_(v0.4 and below: Visual Studio 2013 and Qt 5.5.1)_

## Rules
I try to stick to the following rules, and so should you if you're thinking about submitting a PR. I'm saying "try" because I make mistakes and might deviate from them (knowingly or unknowingly), too.

- This is a Qt/C++ project. Always prefer using "pure Qt" where possible. Do not introduce other libraries/frameworks (e.g. boost). Do not write C-style functions/code where C++ is applicable. Do not use low-level system APIs and stuff.
- Use comments to annotate your code where necessary and omit them where they would be trivial. Use C++ style comments, i.e. `//`, whenever possible.
- Test your code before submitting a PR.
- Try to adapt my coding style: indentation, bracket placing, naming, ...  so that your code fits nicely into the existing codebase and doesn't feel or look foreign at first sight.
- Follow [the seven rules of great git commit messages](http://chris.beams.io/posts/git-commit/).

All in all, it's not that bad, isn't it?

#### You might ask...
- why the existing codebase doesn't fully comply with those rules.  
:arrow_right: That's one of the reasons why we're still at 0.x releases. It's a work in progress.
- why I still changed your code although you followed all the rules.  
:arrow_right: Don't be offended. I frequently rename things or move stuff around to adjust it to my liking. I do this all the time, not only to your code.

# Contributing ...?
... other stuff? ... We'll figure this out once we get there.

# Submitting bug reports
You found a :bug:? Open an [issue](https://github.com/dehesselle/dayzsrvip/issues). Include your TeamSpeak Client's log (menu `Tools`, `Client Log` or press `<CTRL>+<L>`).
