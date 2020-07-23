# Themer

Tool to manage many themes of ~/.config/ files

## Installing

Run

	make install

** note that at the moment, themer gets installed to my personal ~/scripts/ directory, since it is not ready for nor anticipated that others will use it yet

To change the install directory, edit makefile and modify the INSTALL_DIR variable

## Purpose

Themer is used to quickly switch between many slightly different versions of files. For example, if your i3 setup has a light and a dark theme, you can create one i3.config file with `{THEME:DARK}` and `{THEME:LIGHT}` headers (ended using the `{THEME:ENDTHEME}` header), and running `themer dark` or `themer light` will automatically create an i3 config using only the respective sections

## Tutorial

Themer creates the following directories and files:
* `~/.config/themer/`
* `~/.config/themer/src/`
* `~/.config/themer/bin/`
* `~/.config/themer/themer-post.sh`

`~/.config/themer/src/` contains all tracked configs, including their `{THEME}` headers

Running `themer THEME` writes the themed versions to `~/.config/themer/bin/`

Symlinks created by the user link the themed `~/.config/themer/bin/` versions to their respective programs' config directory

Any necessary commands to restart the config's program can be added to `~/.config/themer/themer-post.sh`, which gets called once the last config has been themed and written to `~/.config/themer/bin/`

## Example

1. Create `~/.config/themer/src/example.config`:

		I am an example! My theme is:
		{THEME:DARK}
		dark
		{THEME:LIGHT}
		light
		{THEME:ENDTHEME}

2. Run

		ln -s ~/.config/themer/bin/example.config ~/.config/example/config

3. Add the necessary reset commands to `~/.config/themer/themer-post.sh`:

		killall example
		example

4. Run `themer dark` to:

	a. Create `~/.config/themer/bin/example.config`:
	
		I am an example! My theme is:
		dark

	b. Automatically call `~/.config/themer/themer-post.sh`

## Headers

Themer supports several headers, which take the format {HEADER:VALUE}

* THEME
	- Specifies a new theme section
	- Lines within THEME headers will only be preserved if the value matches the argument passed at runtime
	- The THEME header must be alone on its line, aside from whitespace.
* COLOR
	- Gets replaced with its definition in the colors.ini file
