This document describes the general rules/guidelines you'll have to follow if you're thinking about contributing. The following is not meant to sound harsh or arrogant, just as plain and simple as possible: __my project, my rules__.

You don't like what you're reading here? Remember that you can always stay within your fork where you can do what you want, the way you want.

Last, but not least: I am not Linus Torvalds. You can talk to me and I know that there's another human being on the other side, too.

# Contributing code

## Tools
I use Visual Studio Community 2015 and Qt 5.6.1, I suggest you do the same. Install Visual Studio first, then Qt. Qt Creator will automatically detect MSVC2015, no additional configuration required. Works pretty much out-of-the-box :+1:.

_(v0.4 and below: Visual Studio 2013 and Qt 5.5.1)_

## Rules
I try to stick to the following rules and so should you. I'm saying "try" because I make mistakes and deviate from them (knowingly or unknowingly), too, and because it's a work in progress.

- This is a Qt/C++ project. Always prefer using "pure Qt" where possible. Do not introduce other libraries/frameworks (e.g. boost) or write C-style functions using low-level system calls and stuff.
- Try to adapt to my coding style: indentation, bracket placing, naming, ...  so that your code fits nicely into the existing codebase and doesn't feel or look foreign at first sight. But don't be offended if I still end up renaming things and moving stuff around (because: taste) - I do this all the time, not only to your code.
- Do not submit code you haven't tested. I am not here to test your code.
- Do not even think about submitting code you haven't even compiled yet. If I spot syntax errors reviewing your code, you'll have a hard time convincing me to ever consider looking at your stuff again.
- Follow [the seven rules of great git commit messages](http://chris.beams.io/posts/git-commit/).

All in all, it's not that bad, isn't it?

# Contributing ...?
... other stuff? ... We'll figure this out once we get there.
