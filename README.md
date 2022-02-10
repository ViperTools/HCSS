<p align="center">
  <img src="/HCSS%20Logo.svg" width="50%">
</p>
<p align="center"><a href="https://github.com/ViperTools/HCSS-Public">Looking for documentation?</a></p>

This is the source code for the HCSS transpiler. It consists of 3 main parts, not including utilities like the testing framework. First is the lexer, which takes in the HCSS source code and tokenizes it. Next is the parser, which takes in tokens from the lexer and produces a list of syntax nodes. Lastly we have the transpiler, which takes in the nodes from the parser and produces regular CSS and possibly JS. If you want more information on how the HCSS source code works, visit the [under the hood](https://github.com/ViperTools/HCSS-Public/wiki/Under-the-Hood) section in the HCSS wiki.

# Getting Started
Using HCSS is very simple. It uses an easy to use configuration format and allows you to get up and running with minimal hassle.
## Example Configuration File
