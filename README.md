<p align="center">
  <img src="/HCSS%20Logo.svg" width="50%">
</p>
<p align="center">HCSS (Hydra CSS) is a modified (and hopefully improved) version of CSS that provides extra functionality and syntax shortcuts. It is not like most other CSS preprocessors. HCSS does not allow blatantly invalid syntax, and is not as forgiving in many aspects. This allows HCSS to stay readable and clean across projects.</p>

# Rules
- Invalid syntax is NOT allowed. ...
- Nesting requires the nesting selector (&) before each selector. This enhances readability and allows people to quickly spot where extra rules are.
- Unlike other similar projects, variables must be defined before they are used. This keeps the code organized and accessible to other developers.

# Features
- [x] Nesting
- [ ] Custom variable syntax
- [ ] Events
- - [ ] $EVAL
- - [ ] Toggle syntax (a | b)
- [ ] Custom media queries
- [ ] Themes

# Development
- [x] Lexer
- [x] Parser
- [x] Transpiler
- [x] Switch some grammar elements to structs over pairs to improve readability and extendibility
- [ ] Add Doxygen comments to improve readability
- [ ] Optimize as much as possible
