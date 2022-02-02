<p align="center">
  <img src="/HCSS%20Logo.svg" width="50%">
</p>
<p align="center">HCSS (Hydra CSS) is a modified (and hopefully improved) version of CSS that provides extra functionality and syntax shortcuts. It is not like most other CSS preprocessors. HCSS does not allow blatantly invalid syntax, and is not as forgiving in many aspects. This allows HCSS to stay readable and consistent across projects.</p>

# Notes
- Invalid syntax is NOT allowed. Normally CSS parsers just skip invalid syntax, but I don't think there is really a reason to use invalid syntax in HCSS. The HCSS parser will error and notify you if there is invalid syntax.
- Nesting requires the nesting selector `&` before each selector. This enhances readability and allows people to quickly spot where extra rules are.
- Unlike other similar projects, variables and custom media queries must be defined before they are used. This keeps the code organized and accessible to other developers.

# Features
## Nesting Selector
Like most CSS preprocessors, HCSS has nesting capabilities. Nesting can be achieved with the nesting selector `&`. You can (in theory) nest infinitely, but for the sake of readability, it is recommended not to over nest styles.
### HCSS
```css
one, two {
  color: red;
  &three {
    color: blue;
  }
}
```
### CSS
```css
one, two {
  color: red;
}

:is(one, two) three {
  color: blue;
}
```
Because of the way the nesting selector is parsed, it can also be used as a parent selector.
### HCSS
```css
one, two {
  &:hover {
    color: red;
  }
}
```
### CSS
```css
:is(one, two):hover {
  color: red;
}
```

## Variables
HCSS has variables that can be declared at the top level. They are terminated at a line break or semicolon unless the value is a block. The syntax for variables is `$name = value`.
### HCSS
```css
$var = #FFF;

p {
  color: $var;
}
```
### CSS
```css
p {
  color: #FFF;
}
```

## Custom Media Queries
HCSS allows you to define your own media queries and use them later. Like variables, a custom media query must be declared before use.
### HCSS
```css
@mobile only screen and (max-width: 600px);

@mobile {
  p {
    color: red;
  }
}
```
### CSS
```css
@media only screen and (max-width: 600px) {
  p {
    color: red;
  }
}
```

## Events
HCSS adds events to elements using the pseudo selector syntax. They are pretty useless on their own, but when used in combination with the [toggle operator](#toggle-operator), events are extremely powerful.
### HCSS
```css
button:click {
  background-color: red;
}
```
### CSS & JS
```css
.button-click {
  background-color: red;
}
```
```js
document.querySelectorAll('button').forEach(e => {
  e.addEventListener('click', () => {
    e.classList.add('button-click');
  })
})
```

## Toggle Operator
The toggle operator `|` is used to switch between property values in events.
### HCSS
```css
button a:click {
  background-color: red | blue | green;
  color: white | black;
}
```
### CSS & JS
```css
.button-a-click1 {
  background-color: red;
  color: white;
}

.button-a-click2 {
  background-color: blue;
  color: black;
}

.button-a-click3 {
  background-color: green;
  color: white;
}
```
```js
document.querySelectorAll('button a').forEach(e => {
  let i = 1, max = 3;
  e.addEventListener('click', () => {
    e.classList.remove(`button-a-click${i - 1}`);
    if (i > max) i = 1;
    e.classList.add(`button-a-click${i}`);
    i++;
  })
})
```

## JavaScript Eval
HCSS allows you to use JavaScript code right in your CSS stylesheet using `$EVAL`. It can only be used in events or at the top level. `$EVAL` can take a list of parameters if inside of an event.
###  HCSS
```css
button:click {
  $EVAL(ev) {
    console.log('I have been clicked!', ev);
  }
}
```
### JS
```js
document.querySelectorAll('button').forEach(e => {
  e.addEventListener('click', (ev) => {
    console.log('I have been clicked!', ev);
  })
})
```

## Themes
Themes allow you to easily implement custom themes without writing any JavaScript. Themes can be created using `@theme`. The `theme` property can also be used with the [toggle operator](#toggle-operator) to toggle the theme with an event. There is also a `global-theme` property that will apply the theme to the body element.
### HCSS
```css
@theme dark {
  background: black;
  color: white;
}

p {
  theme: dark;
}
```
### CSS
```css
p {
  background: black;
  color: white;
}
```

# Development
- [x] Lexer
- [x] Parser
- [x] Transpiler
- [x] Switch some grammar elements to structs over pairs to improve readability and extendibility
- [x] Nesting
- [ ] Custom variable syntax
- [ ] Events
- - [ ] $EVAL
- - [ ] Toggle syntax (a | b)
- [ ] Custom media queries
- [ ] Themes
- [ ] Add Doxygen comments to improve readability
- [ ] Optimize as much as possible
