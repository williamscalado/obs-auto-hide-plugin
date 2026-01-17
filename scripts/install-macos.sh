#!/bin/bash

# Install script for macOS
echo "Installing Auto Hide Scenes (macOS Bundle)..."

# Define variables
PLUGIN_NAME="auto-hide-scenes"
# Path to user plugins directory
PLUGINS_DIR="$HOME/Library/Application Support/obs-studio/plugins"
# The .plugin bundle directory
BUNDLE_PATH="$PLUGINS_DIR/${PLUGIN_NAME}.plugin"
# Inside the bundle
MACOS_DIR="$BUNDLE_PATH/Contents/MacOS"
RESOURCES_DIR="$BUNDLE_PATH/Contents/Resources"

# Clean old installation
rm -rf "$BUNDLE_PATH"
# Remove potential duplicate installations (legacy folders or loose files)
rm -rf "$PLUGINS_DIR/$PLUGIN_NAME" 
rm -f "$PLUGINS_DIR/lib$PLUGIN_NAME.so"
rm -f "$PLUGINS_DIR/$PLUGIN_NAME.so"

# Create directories
mkdir -p "$MACOS_DIR"
mkdir -p "$RESOURCES_DIR"

# Copy binary and rename (remove 'lib' prefix and '.so' extension for bundle standard)
# Note: OBS on Mac can load .so, but inside a bundle typically it matches the bundle name
cp build/libauto-hide-scenes.so "$MACOS_DIR/$PLUGIN_NAME"

# Target for patching
TARGET_LIB="$MACOS_DIR/$PLUGIN_NAME"

echo "Patching Qt dependencies on $TARGET_LIB..."
# Change absolute Homebrew paths to relative @rpath (to use OBS's bundled Qt)
install_name_tool -change "/opt/homebrew/opt/qtbase/lib/QtCore.framework/Versions/A/QtCore" "@rpath/QtCore.framework/Versions/A/QtCore" "$TARGET_LIB"
install_name_tool -change "/opt/homebrew/opt/qtbase/lib/QtGui.framework/Versions/A/QtGui" "@rpath/QtGui.framework/Versions/A/QtGui" "$TARGET_LIB"
install_name_tool -change "/opt/homebrew/opt/qtbase/lib/QtWidgets.framework/Versions/A/QtWidgets" "@rpath/QtWidgets.framework/Versions/A/QtWidgets" "$TARGET_LIB"
install_name_tool -change "/opt/homebrew/opt/qtbase/lib/QtNetwork.framework/Versions/A/QtNetwork" "@rpath/QtNetwork.framework/Versions/A/QtNetwork" "$TARGET_LIB"

# Copy data
cp -r data/locale "$RESOURCES_DIR/"

# Create Info.plist (Optional but good for bundles)
cat > "$BUNDLE_PATH/Contents/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>$PLUGIN_NAME</string>
    <key>CFBundleIdentifier</key>
    <string>com.example.$PLUGIN_NAME</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>$PLUGIN_NAME</string>
    <key>CFBundlePackageType</key>
    <string>BNDL</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundleVersion</key>
    <string>1</string>
</dict>
</plist>
EOF

# Permissions
chmod +x "$TARGET_LIB"

# Remove quarantine (fixes "damaged" or security errors)
xattr -d com.apple.quarantine "$TARGET_LIB" 2>/dev/null

echo "Installation complete!"
echo "Installed to: $BUNDLE_PATH"