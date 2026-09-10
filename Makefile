CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -Iinclude -Isrc
LDFLAGS ?= -lwayland-client -lwayland-egl -l:libEGL.so.1 -l:libmpv.so.2 -l:libGL.so.1

PREFIX ?= $(HOME)/.local
BINDIR = $(PREFIX)/bin
AUTOSTART_DIR = $(HOME)/.config/autostart
SYSTEMD_USER_DIR = $(HOME)/.config/systemd/user

PROTOCOL_XML = protocols/wlr-layer-shell-unstable-v1.xml
PROTOCOL_H = src/wlr-layer-shell-protocol.h
PROTOCOL_C = src/wlr-layer-shell-protocol.c

ENGINE_BIN = bin/cosmic-wallpaper-engine
DAEMON_BIN = bin/cosmic-wallpaper-daemon
CLIENT_BIN = bin/cosmic-wallpaper-client
UI_BIN = bin/cosmic-wallpaper-ui
APPS_DIR = $(HOME)/.local/share/applications

.PHONY: all clean install uninstall protocol

all: $(ENGINE_BIN)

protocol: $(PROTOCOL_H) $(PROTOCOL_C)

$(PROTOCOL_H): $(PROTOCOL_XML)
	wayland-scanner client-header $< $@

$(PROTOCOL_C): $(PROTOCOL_XML)
	wayland-scanner private-code $< $@

$(ENGINE_BIN): src/engine.c $(PROTOCOL_C) $(PROTOCOL_H)
	@mkdir -p bin
	$(CC) $(CFLAGS) src/engine.c $(PROTOCOL_C) -o $@ $(LDFLAGS)
	@chmod +x $(DAEMON_BIN) $(CLIENT_BIN) $(UI_BIN) run.sh

clean:
	rm -f $(ENGINE_BIN)

install: all
	@mkdir -p $(BINDIR)
	@mkdir -p $(AUTOSTART_DIR)
	@mkdir -p $(SYSTEMD_USER_DIR)
	@mkdir -p $(APPS_DIR)
	install -m 755 $(ENGINE_BIN) $(BINDIR)/cosmic-wallpaper-engine
	install -m 755 $(DAEMON_BIN) $(BINDIR)/cosmic-wallpaper-daemon
	install -m 755 $(CLIENT_BIN) $(BINDIR)/cosmic-wallpaper-client
	install -m 755 $(UI_BIN) $(BINDIR)/cosmic-wallpaper-ui
	@sed 's|Exec=cosmic-wallpaper-daemon|Exec=$(BINDIR)/cosmic-wallpaper-daemon|g' autostart/cosmic-wallpaper.desktop > $(AUTOSTART_DIR)/cosmic-wallpaper.desktop
	@sed 's|ExecStart=cosmic-wallpaper-daemon|ExecStart=$(BINDIR)/cosmic-wallpaper-daemon|g' systemd/cosmic-wallpaper.service > $(SYSTEMD_USER_DIR)/cosmic-wallpaper.service
	@sed 's|Exec=cosmic-wallpaper-ui|Exec=$(BINDIR)/cosmic-wallpaper-ui|g' desktop/io.github.AlanDrone.animated-wallpaper.desktop > $(APPS_DIR)/io.github.AlanDrone.animated-wallpaper.desktop
	@echo "✓ Successfully installed to $(BINDIR)"
	@echo "✓ App launcher created in $(APPS_DIR)"
	@echo "✓ Autostart created in $(AUTOSTART_DIR)/cosmic-wallpaper.desktop"
	@echo "✓ Systemd user service created in $(SYSTEMD_USER_DIR)/cosmic-wallpaper.service"

uninstall:
	rm -f $(BINDIR)/cosmic-wallpaper-engine
	rm -f $(BINDIR)/cosmic-wallpaper-daemon
	rm -f $(BINDIR)/cosmic-wallpaper-client
	rm -f $(BINDIR)/cosmic-wallpaper-ui
	rm -f $(APPS_DIR)/io.github.AlanDrone.animated-wallpaper.desktop
	rm -f $(AUTOSTART_DIR)/cosmic-wallpaper.desktop
	rm -f $(SYSTEMD_USER_DIR)/cosmic-wallpaper.service
	@echo "✓ Successfully uninstalled cosmic-wallpaper"
