# Themer

Tool to manage many themes of ~/.config/ files.

## Installing

Run

	make install

** note that at the moment, themer gets installed to my personal ~/scripts/ directory, since it is not ready for nor anticipated that others will use it yet.

## Purpose

Themer is used to quickly switch between many slightly different versions of files. For example, if your i3 setup has a light and a dark theme, you can create one i3.config file with {THEME:DARK} and {THEME:LIGHT} headers (ended using the {THEME:ENDTHEME} header), and running `themer dark` or `themer light` will automatically create an i3 config using only the respective sections.

## Tutorial

Themer creates the following directories and files:
* `~/.config/themer/`
* `~/.config/themer/src/`
* `~/.config/themer/bin/`
* `~/.config/themer/themer-post.sh`

The intent is that all config files will be moved to `~/.config/themer/src/`, which will output the themed versions to `~/.config/themer/bin/`. Symlinks will then be created by

## Example

1. Create ~/.config/themer/src/example.config:

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
