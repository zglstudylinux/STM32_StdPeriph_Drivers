#!/usr/bin/env bash
# ==============================================
#  STM32 工具链环境检测 & 配置脚本
#  自动检测 arm-gcc, openocd, make, python
#  生成 tools_paths.conf 供其他脚本引用
# ==============================================

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CONF_FILE="${SCRIPT_DIR}/tools_paths.conf"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

pass() { echo -e "  ${GREEN}[OK]${NC}    $1"; }
fail() { echo -e "  ${RED}[MISS]${NC} $1"; }
warn() { echo -e "  ${YELLOW}[WARN]${NC} $1"; }
info() { echo -e "  ${CYAN}[INFO]${NC} $1"; }

MISSING=0
ARM_GCC_FOUND=""
OPENOCD_FOUND=""
OPENOCD_SCRIPTS=""
MAKE_FOUND=""
PYTHON_FOUND=""

echo "========================================"
echo "  STM32 工具链检测"
echo "========================================"
echo ""

# ==================== Python ====================
echo "--- Python ---"

# Search for real Python (skip WindowsApps stubs that don't actually work)
PYTHON_SEARCH=(
    "command:python3"
    "command:python"
    "/c/ProgramData/Anaconda3/python"
    "/c/Python3*/python"
    "/c/Users/*/AppData/Local/Programs/Python/Python3*/python"
    "/usr/bin/python3"
    "/usr/local/bin/python3"
)

PYTHON_FOUND=""
for entry in "${PYTHON_SEARCH[@]}"; do
    if [[ "$entry" == command:* ]]; then
        cmd="${entry#command:}"
        candidate=$(command -v "$cmd" 2>/dev/null || echo "")
        if [ -n "$candidate" ]; then
            # Skip WindowsApps stubs (they don't work from bash)
            if [[ "$candidate" == *"/WindowsApps/"* ]]; then
                continue
            fi
            # Test if it actually runs
            if "$candidate" --version >/dev/null 2>&1; then
                PYTHON_FOUND="$candidate"
                break
            fi
        fi
    else
        for f in $entry; do
            if [ -f "$f" ] || [ -f "$f.exe" ]; then
                candidate="${f}"
                [ -f "$f.exe" ] && candidate="${f}.exe"
                if "$candidate" --version >/dev/null 2>&1; then
                    PYTHON_FOUND="$candidate"
                    break 2
                fi
            fi
        done
    fi
done

if [ -n "$PYTHON_FOUND" ]; then
    PY_VER=$("$PYTHON_FOUND" --version 2>&1)
    pass "$PY_VER  ($PYTHON_FOUND)"

    # Check pyserial
    if "$PYTHON_FOUND" -c "import serial" 2>/dev/null; then
        pass "pyserial module"
    else
        warn "pyserial not installed — run: pip install pyserial"
    fi

    # Check tkinter
    if "$PYTHON_FOUND" -c "import tkinter" 2>/dev/null; then
        pass "tkinter module"
    else
        warn "tkinter not installed — run: pip install tkinter"
    fi
else
    fail "Python 3.x"
    info "Download: https://www.python.org/downloads/"
    MISSING=$((MISSING+1))
fi
echo ""

# ==================== ARM GCC ====================
echo "--- ARM GCC Toolchain ---"

# Search order: PATH > common Windows paths > common Linux paths
SEARCH_PATHS=(
    # PATH first
    "PATH:arm-none-eabi-gcc"
    # Windows common paths
    "/c/tools/arm-gcc/bin"
    "/c/Program Files (x86)/GNU Arm Embedded Toolchain/*/bin"
    "/c/Program Files/GNU Arm Embedded Toolchain/*/bin"
    "/c/ProgramData/chocolatey/bin"
    # Linux common paths
    "/usr/bin"
    "/usr/local/bin"
    "/opt/gcc-arm-none-eabi-*/bin"
)

for sp in "${SEARCH_PATHS[@]}"; do
    if [ "$sp" = "PATH:arm-none-eabi-gcc" ]; then
        ARM_GCC_FOUND=$(command -v arm-none-eabi-gcc 2>/dev/null || echo "")
    else
        # Expand wildcards
        for dir in $sp; do
            if [ -f "$dir/arm-none-eabi-gcc" ] || [ -f "$dir/arm-none-eabi-gcc.exe" ]; then
                ARM_GCC_FOUND="$dir/arm-none-eabi-gcc"
                break 2
            fi
        done
    fi
    [ -n "$ARM_GCC_FOUND" ] && break
done

if [ -n "$ARM_GCC_FOUND" ]; then
    ARM_GCC_DIR=$(dirname "$ARM_GCC_FOUND")
    GCC_VER=$("$ARM_GCC_FOUND" --version 2>&1 | head -1)
    pass "$GCC_VER"
    pass "Location: $ARM_GCC_DIR"
else
    fail "arm-none-eabi-gcc"
    info "Download: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads"
    MISSING=$((MISSING+1))
fi
echo ""

# ==================== OpenOCD ====================
echo "--- OpenOCD ---"

OPENOCD_SEARCH=(
    "PATH:openocd"
    # xPack OpenOCD
    "/c/tools/openocd/xpack-openocd-0.12.0-3/bin"
    "/c/tools/openocd/xpack-openocd-0.12.0-2/bin"
    "/c/tools/openocd/xpack-openocd-0.11.0-1/bin"
    "/c/tools/openocd/*/bin"
    # Chocolatey
    "/c/ProgramData/chocolatey/bin"
    # Other Windows
    "/c/OpenOCD/bin"
    # Linux
    "/usr/bin"
    "/usr/local/bin"
)

for sp in "${OPENOCD_SEARCH[@]}"; do
    if [ "$sp" = "PATH:openocd" ]; then
        OPENOCD_FOUND=$(command -v openocd 2>/dev/null || echo "")
        [ -n "$OPENOCD_FOUND" ] && break
    else
        for dir in $sp; do
            if [ -f "$dir/openocd" ] || [ -f "$dir/openocd.exe" ]; then
                OPENOCD_FOUND="$dir/openocd"
                break 2
            fi
        done
    fi
done

if [ -n "$OPENOCD_FOUND" ]; then
    OCD_BIN_DIR="$(dirname "$OPENOCD_FOUND")"

    # Find scripts directory (try multiple levels)
    OPENOCD_SCRIPTS=""
    for scripts_candidate in \
        "${OCD_BIN_DIR}/../scripts" \
        "${OCD_BIN_DIR}/../openocd/scripts" \
        "${OCD_BIN_DIR}/../../scripts" \
        "${OCD_BIN_DIR}/../share/openocd/scripts" \
        "/c/tools/openocd/xpack-openocd-0.12.0-3/openocd/scripts" \
        "/c/tools/openocd/xpack-openocd-0.12.0-3/scripts" \
        "/c/tools/openocd/xpack-openocd-0.12.0-2/openocd/scripts" \
        "/usr/share/openocd/scripts" \
        "/usr/local/share/openocd/scripts"; do
        if [ -d "$scripts_candidate" ] && [ -f "$scripts_candidate/interface/stlink.cfg" ]; then
            OPENOCD_SCRIPTS="$(cd "$scripts_candidate" 2>/dev/null && pwd || echo "$scripts_candidate")"
            break
        fi
    done

    OCD_VER=$("$OPENOCD_FOUND" --version 2>&1 | head -1 || echo "unknown version")
    pass "$OCD_VER"
    if [ -n "$OPENOCD_SCRIPTS" ]; then
        pass "Scripts: $OPENOCD_SCRIPTS"
    else
        warn "Scripts directory not found — set OPENOCD_SCRIPTS manually"
    fi
else
    fail "openocd"
    info "Download: https://gnutoolchains.com/arm-eabi/openocd/"
    info "Or: https://github.com/xpack-dev-tools/openocd-xpack/releases"
    MISSING=$((MISSING+1))
fi
echo ""

# ==================== Make ====================
echo "--- Make ---"
MAKE_FOUND=$(command -v make 2>/dev/null || echo "")
if [ -z "$MAKE_FOUND" ]; then
    [ -f "/c/tools/make/bin/make.exe" ] && MAKE_FOUND="/c/tools/make/bin/make.exe"
    [ -f "/c/tools/make/make.exe" ] && MAKE_FOUND="/c/tools/make/make.exe"
fi

if [ -n "$MAKE_FOUND" ]; then
    MAKE_VER=$("$MAKE_FOUND" --version 2>&1 | head -1 || echo "make")
    pass "$MAKE_VER  ($MAKE_FOUND)"
else
    warn "make not found (optional — build scripts work without it)"
    info "Download: https://sourceforge.net/projects/mingw/"
fi
echo ""

# ==================== ST-Link ====================
echo "--- ST-Link ---"
STLINK_FOUND=$(command -v st-info 2>/dev/null || echo "")
if [ -n "$STLINK_FOUND" ]; then
    pass "st-info available ($STLINK_FOUND)"
else
    warn "st-info not found (optional — OpenOCD handles flashing)"
fi
echo ""

# ==================== Generate Config ====================
echo "========================================"
echo ""

if [ $MISSING -gt 0 ]; then
    echo -e "${RED}${MISSING} required tool(s) missing!${NC}"
    echo "Please install the missing tools above and re-run this script."
    echo ""
fi

if [ -z "$ARM_GCC_FOUND" ]; then
    echo -e "${RED}Cannot generate config: ARM GCC is required.${NC}"
    exit 1
fi

# Write config file
cat > "${CONF_FILE}" << EOF
# Generated by setup_env.sh — $(date)
# Source this file in build scripts to get toolchain paths
# Re-run setup_env.sh if you move the tools

# ARM GCC
ARM_GCC_DIR="${ARM_GCC_DIR}"
ARM_GCC="${ARM_GCC_FOUND}"

# OpenOCD
OPENOCD="${OPENOCD_FOUND:-}"
OPENOCD_SCRIPTS="${OPENOCD_SCRIPTS:-}"

# Make
MAKE="${MAKE_FOUND:-}"

# Python
PYTHON="${PYTHON_FOUND:-}"

# Project root
PROJECT_ROOT="${PROJECT_ROOT}"

# STM32 serial port (change if different)
SERIAL_PORT="\${STM32_SERIAL_PORT:-COM3}"
SERIAL_BAUD="\${STM32_SERIAL_BAUD:-115200}"
EOF

echo -e "${GREEN}Configuration saved to:${NC}"
echo "  ${CONF_FILE}"
echo ""
echo -e "${CYAN}Usage in other scripts:${NC}"
echo "  source tools/setup_env.sh    # Auto-detect and generate config"
echo "  # Or to just check status:"
echo "  bash tools/setup_env.sh      # Run standalone"
echo ""

if [ $MISSING -eq 0 ]; then
    echo -e "${GREEN}All required tools found! Ready to build.${NC}"
fi

exit $MISSING
