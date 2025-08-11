#!/usr/bin/env bash
set -e

echo "🔍 Checking for dependencies..."

# Check for pkg-config
if ! command -v pkg-config >/dev/null 2>&1; then
    echo "❌ pkg-config is not installed."
    echo "   Install it first:"
    echo "     Ubuntu/Debian: sudo apt-get install pkg-config"
    echo "     macOS (brew):  brew install pkg-config"
    echo "     CentOS/RHEL:   sudo yum install pkgconf-pkg-config"
    exit 1
fi

missing=()

# Check readline
if pkg-config --exists readline; then
    echo "✅ readline found"
else
    echo "❌ readline not found"
    missing+=("readline")
fi

# Check mpfr
if pkg-config --exists mpfr; then
    echo "✅ mpfr found"
else
    echo "❌ mpfr not found"
    missing+=("mpfr")
fi

# Install hints if missing
if [ ${#missing[@]} -gt 0 ]; then
    echo
    echo "The following dependencies are missing: ${missing[*]}"
    echo "You can install them using one of the following commands:"
    echo
    echo "  Ubuntu/Debian: sudo apt-get install ${missing[*]/mpfr/libmpfr-dev} ${missing[*]/readline/libreadline-dev} libgmp-dev"
    echo "  macOS (brew):  brew install ${missing[*]}"
    echo "  CentOS/RHEL:   sudo yum install ${missing[*]/mpfr/mpfr-devel} ${missing[*]/readline/readline-devel} gmp-devel"
    echo
    exit 1
fi

mkdir -p obj include bin

echo "✅ All dependencies found!"
echo "You can now run: make"

