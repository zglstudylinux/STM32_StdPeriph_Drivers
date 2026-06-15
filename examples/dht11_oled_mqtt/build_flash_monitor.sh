#!/usr/bin/env bash
# ==============================================
#  STM32 编译-烧录-串口监控 一键脚本
#  For DHT11 + OLED + MQTT project
#
#  首次使用请先运行:
#    bash tools/setup_env.sh
#  自动检测工具链后即可直接使用本脚本
# ==============================================
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TOOLS_DIR="$(cd "${SCRIPT_DIR}/../../tools" && pwd)"
CONF_FILE="${TOOLS_DIR}/tools_paths.conf"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

info()  { echo -e "${GREEN}[INFO]${NC}  $*"; }
warn()  { echo -e "${YELLOW}[WARN]${NC} $*"; }
error() { echo -e "${RED}[ERROR]${NC} $*"; }
step()  { echo -e "\n${CYAN}====== $* ======${NC}"; }

# ==================== Toolchain Detection ====================
detect_toolchain() {
    # Try to load cached config first
    if [ -f "${CONF_FILE}" ]; then
        source "${CONF_FILE}"
        # Verify the paths are still valid
        if [ -n "${ARM_GCC_DIR}" ] && [ -x "${ARM_GCC_DIR}/arm-none-eabi-gcc" ] || [ -x "${ARM_GCC_DIR}/arm-none-eabi-gcc.exe" ]; then
            return 0
        fi
        warn "Cached config outdated, re-detecting..."
    fi

    # Auto-detect: run setup_env.sh
    if [ -f "${TOOLS_DIR}/setup_env.sh" ]; then
        info "First run — detecting toolchain..."
        bash "${TOOLS_DIR}/setup_env.sh" --quiet 2>/dev/null || bash "${TOOLS_DIR}/setup_env.sh"
        if [ -f "${CONF_FILE}" ]; then
            source "${CONF_FILE}"
        fi
    fi

    # Fallback: try PATH
    if [ -z "${ARM_GCC_DIR}" ]; then
        ARM_GCC_DIR=$(dirname "$(command -v arm-none-eabi-gcc 2>/dev/null)" 2>/dev/null || echo "")
    fi

    if [ -z "${OPENOCD}" ]; then
        OPENOCD=$(command -v openocd 2>/dev/null || echo "")
    fi

    # Final check
    if [ -z "${ARM_GCC_DIR}" ] || [ ! -x "${ARM_GCC_DIR}/arm-none-eabi-gcc" ] && [ ! -x "${ARM_GCC_DIR}/arm-none-eabi-gcc.exe" ]; then
        echo ""
        error "ARM GCC not found!"
        echo ""
        echo "  Please run setup first:"
        echo "    bash tools/setup_env.sh"
        echo ""
        echo "  Or install ARM GCC from:"
        echo "    https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads"
        echo ""
        exit 1
    fi
}

# --- Usage ---
usage() {
    cat <<EOF
Usage: $0 [OPTIONS]

Options:
    compile      Only compile
    flash        Compile + flash via ST-Link
    monitor      Compile + flash + serial monitor
    all          Compile + flash + host GUI + serial monitor
    clean        Remove build artifacts

Environment variables:
    STM32_SERIAL_PORT   Serial port (default: COM3)
    STM32_SERIAL_BAUD   Baud rate (default: 115200)

First-time setup:
    bash tools/setup_env.sh     # Auto-detect toolchain

Examples:
    $0                  # Compile only
    $0 flash            # Compile and flash
    SERIAL_PORT=COM5 $0 monitor
EOF
    exit 0
}

# --- Compile ---
do_compile() {
    step "Compiling project"

    if [ ! -d "${ARM_GCC_DIR}" ]; then
        error "ARM GCC directory not found: ${ARM_GCC_DIR}"
        exit 1
    fi

    CC="${ARM_GCC_DIR}/arm-none-eabi-gcc"
    AS="${ARM_GCC_DIR}/arm-none-eabi-as"
    OBJCOPY="${ARM_GCC_DIR}/arm-none-eabi-objcopy"

    # Use .exe extension on Windows
    [ -f "${CC}.exe" ] && CC="${CC}.exe"
    [ -f "${AS}.exe" ] && AS="${AS}.exe"
    [ -f "${OBJCOPY}.exe" ] && OBJCOPY="${OBJCOPY}.exe"

    if [ ! -x "${CC}" ] && [ ! -f "${CC}" ]; then
        error "Compiler not found: ${CC}"
        exit 1
    fi

    GCC_VER=$("${CC}" --version 2>&1 | head -1)
    info "Compiler: ${GCC_VER}"

    cd "${SCRIPT_DIR}"

    CFLAGS="-mcpu=cortex-m3 -mthumb -Wall -Wextra -Wno-missing-braces"
    CFLAGS="${CFLAGS} -ffunction-sections -fdata-sections -Os -std=c99"
    CFLAGS="${CFLAGS} -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER"
    CFLAGS="${CFLAGS} -IStart -ILibrary -IUser -ISystem"

    LDFLAGS="-mcpu=cortex-m3 -mthumb -Tstm32f103c8t6.ld -nostartfiles"
    LDFLAGS="${LDFLAGS} -specs=nano.specs -specs=nosys.specs -Wl,--gc-sections"

    # Compile all C sources
    local objs=()
    local sources=(
        Start/system_stm32f10x.c
        Library/stm32f10x_gpio.c
        Library/stm32f10x_rcc.c
        Library/misc.c
        Library/stm32f10x_usart.c
        Library/stm32f10x_i2c.c
        System/usart.c
        System/dht11.c
        System/oled.c
        System/esp8266.c
        System/delay.c
        User/main.c
        User/stm32f10x_it.c
    )

    for src in "${sources[@]}"; do
        obj="${src%.c}.o"
        printf "  CC   %s\n" "${src}"
        "${CC}" ${CFLAGS} -c "${src}" -o "${obj}"
        objs+=("${obj}")
    done

    # Assemble startup file
    printf "  AS   Start/startup_stm32f10x_md_gcc.s\n"
    "${AS}" -mcpu=cortex-m3 -mthumb Start/startup_stm32f10x_md_gcc.s -o Start/startup_stm32f10x_md_gcc.o
    objs+=("Start/startup_stm32f10x_md_gcc.o")

    # Link
    info "Linking..."
    "${CC}" ${LDFLAGS} -Wl,-Map=Project.map "${objs[@]}" -o Project.elf

    # Generate HEX/BIN
    "${OBJCOPY}" -O ihex Project.elf Project.hex
    "${OBJCOPY}" -O binary Project.elf Project.bin

    info "Build successful!"
    info "  Project.elf  ($(wc -c < Project.elf | tr -d ' ') bytes)"
    info "  Project.hex  ($(wc -c < Project.hex | tr -d ' ') bytes)"
    info "  Project.bin  ($(wc -c < Project.bin | tr -d ' ') bytes)"
}

# --- Flash ---
do_flash() {
    step "Flashing to STM32"

    # Try to find OpenOCD
    if [ -z "${OPENOCD}" ] || [ ! -f "${OPENOCD}" ]; then
        # Search common locations
        for ocd in \
            "/c/tools/openocd/xpack-openocd-0.12.0-3/bin/openocd.exe" \
            "/c/tools/openocd/xpack-openocd-0.12.0-2/bin/openocd.exe" \
            "/c/tools/openocd/*/bin/openocd.exe"; do
            # Expand wildcard
            for f in $ocd; do
                if [ -f "$f" ]; then
                    OPENOCD="$f"
                    OPENOCD_SCRIPTS="$(cd "$(dirname "$f")/../scripts" 2>/dev/null && pwd || echo "")"
                    break 2
                fi
            done
        done
    fi

    if [ -z "${OPENOCD}" ] || [ ! -f "${OPENOCD}" ]; then
        error "OpenOCD not found!"
        echo ""
        echo "  Please install OpenOCD:"
        echo "    https://gnutoolchains.com/arm-eabi/openocd/"
        echo ""
        echo "  Then set OPENOCD in tools/tools_paths.conf"
        echo "  Or re-run: bash tools/setup_env.sh"
        echo ""
        exit 1
    fi

    if [ -z "${OPENOCD_SCRIPTS}" ] || [ ! -d "${OPENOCD_SCRIPTS}" ]; then
        OPENOCD_SCRIPTS="$(cd "$(dirname "${OPENOCD}")/../scripts" 2>/dev/null && pwd || echo "")"
    fi

    if [ ! -f "${SCRIPT_DIR}/Project.hex" ]; then
        error "Project.hex not found. Compile first."
        exit 1
    fi

    info "OpenOCD: ${OPENOCD}"
    info "Scripts: ${OPENOCD_SCRIPTS}"
    info "Please ensure STM32 is connected via ST-Link..."

    cd "${SCRIPT_DIR}"

    if "${OPENOCD}" -s "${OPENOCD_SCRIPTS}" \
        -f interface/stlink.cfg \
        -f stm32f1x_custom.cfg \
        -c "program Project.hex verify reset exit" 2>&1; then
        info "Flash successful! Board is running."
    else
        error "Flash failed."
        echo "  Check:"
        echo "    1. ST-Link USB cable connected"
        echo "    2. STM32 board powered"
        echo "    3. SWD wires (SWCLK/SWDIO/GND) connected"
        exit 1
    fi
}

# --- Serial Monitor ---
do_serial_monitor() {
    step "Starting Serial Monitor"

    local port="${STM32_SERIAL_PORT:-COM3}"
    local baud="${STM32_SERIAL_BAUD:-115200}"

    # Override with SERIAL_PORT if set
    [ -n "${SERIAL_PORT}" ] && port="${SERIAL_PORT}"

    info "Port: ${port}, Baud: ${baud}"
    info "Press Ctrl+C to stop..."

    "${PYTHON:-python}" -c "
import serial, sys, serial.tools.list_ports
port = '${port}'
baud = ${baud}
try:
    ser = serial.Serial(port, baud, timeout=1)
    print(f'[Serial] Connected to {port} @ {baud} baud\\n')
    while True:
        if ser.in_waiting > 0:
            data = ser.read(ser.in_waiting)
            text = data.decode('utf-8', errors='replace')
            print(text, end='', flush=True)
except serial.SerialException as e:
    print(f'[Error] Cannot open {port}: {e}')
    print('\\nAvailable ports:')
    for p in serial.tools.list_ports.comports():
        print(f'  {p.device} - {p.description}')
    sys.exit(1)
except KeyboardInterrupt:
    print('\\n[Serial] Monitor stopped.')
    ser.close()
"
}

# --- Host GUI ---
do_host_gui() {
    step "Starting Host Monitor GUI"
    info "TCP Server listening on port 1883"
    info "Make sure STM32 WiFi config points to this PC's IP"
    "${PYTHON:-python}" "${TOOLS_DIR}/mqtt_monitor.py" &
    HOST_PID=$!
    info "Host GUI started (PID: ${HOST_PID})"
}

# --- Clean ---
do_clean() {
    step "Cleaning build artifacts"
    cd "${SCRIPT_DIR}"
    rm -f Start/*.o Library/*.o System/*.o User/*.o 2>/dev/null
    rm -f Project.elf Project.hex Project.bin Project.map 2>/dev/null
    info "Clean complete."
}

# ==================== Main ====================
detect_toolchain

case "${1:-compile}" in
    compile)
        do_compile
        ;;
    flash)
        do_compile
        do_flash
        ;;
    monitor)
        do_compile
        do_flash
        do_serial_monitor
        ;;
    all)
        do_compile
        do_flash
        do_host_gui
        info "Starting serial monitor in foreground..."
        sleep 2
        do_serial_monitor
        ;;
    clean)
        do_clean
        ;;
    -h|--help|help)
        usage
        ;;
    *)
        echo "Unknown option: $1"
        usage
        ;;
esac
