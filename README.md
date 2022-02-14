<p align="center">
  <img src="/HCSS%20Logo.svg" width="75%">
</p>
<p align="center"><a href="https://github.com/ViperTools/HCSS-Public">Looking for documentation?</a></p>

This is the source code for the HCSS transpiler. It consists of 3 main parts, not including utilities like the testing framework. First is the lexer, which takes in the HCSS source code and tokenizes it. Next is the parser, which takes in tokens from the lexer and produces a list of syntax nodes. Lastly we have the transpiler, which takes in the nodes from the parser and produces regular CSS and possibly JS. If you want more information on how the HCSS source code works, visit the [under the hood](https://github.com/ViperTools/HCSS-Public/wiki/Under-the-Hood) section in the HCSS wiki.

# Getting Started
Using HCSS is very simple. It uses an easy to use configuration format and allows you to get up and running with minimal hassle.<br/>
First, make sure you've installed Hydra. If you deselected the `Add to path` option in the installer, you need to use the path to the hydra executable rather than just typing `hydra`.
## Basic Setup
Create a file named `config.hydra` in the root directory of your project with the following text:
```
hcss
    watch
        delay 500ms
        recursive
        directories
            /
        files
            *.hcss
    output $NAME.css
```
In your terminal run `hydra css`
Since `watch` is configured, it should work automatically. Changes to any `.hcss` file in your project will now create a matching `.css` file in the same directory.

## Advanced Setup
This section covers more advanced configuration and setup for HCSS. If you just want your HCSS files to be transpiled to CSS and JS in the same directory, just follow the [basic setup](#basic-setup).

### Changing the Configuration File Name
Use the command line argument `--config` and the path of your configuration file. e.g `hydra css --config "./config/hcss.conf"`.
> The configuration file must follow the hydra configuration format regardless of the file extension.

### Configuring HCSS
You can configure many aspects of HCSS to fit your needs. You must begin the HCSS configuration with `hcss` due to the fact that Hydra uses a universal configuration file for the entire project.
#### watch
The `watch` option has a few properties and attributes.
- `<time> delay` Configures the amound of time between each check
- `recursive` The recursive attribute allows the file watcher to go through directories inside of the specified directories
- `<path[]> directories` Defines what directories HCSS should check
- `<path[]> files` Defines specific files to watch
#### output
The output property allows you to change the file names of the css files. You can use the `$NAME` variable to get the original file name.
