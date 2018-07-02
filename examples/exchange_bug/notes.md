# Exchange

- Testcase result of pass fail, when reducing other stuff
- Pass fails, when executing "" w/ error message: exchange: Assertion `irg == get_irn_irg(nw)' failed.
- Error occurs when using pass 'pass_replace_loads' (uses exchange function) -- possibly also when using other passes
- first step is to reduce testcase and see what we get, before we further analyse