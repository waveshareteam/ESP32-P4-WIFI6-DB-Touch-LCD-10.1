# SPDX-FileCopyrightText: 2023-2025 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Unlicense OR CC0-1.0
import re

import pytest
from pytest_embedded_idf import IdfDut
from pytest_embedded_idf.utils import idf_parametrize

EXPECT_TIMEOUT = 20
PROMPT = 'i2c-tools>'

# On-board devices on the default I2C bus (SDA = GPIO7, SCL = GPIO8)
ES8389_ADDR = 0x10
PCF85063A_ADDR = 0x51
PCF85063A_RAM_BYTE_REG = 0x03


@pytest.mark.generic
@idf_parametrize('target', ['esp32p4'], indirect=['target'])
def test_i2ctools_example(dut: IdfDut) -> None:
    dut.expect_exact(PROMPT, timeout=EXPECT_TIMEOUT)
    # The bus is already up on the default pins, so scan right away
    dut.write('i2cdetect')
    dut.expect(re.compile(rb'10: 10 '), timeout=EXPECT_TIMEOUT)  # ES8389 audio codec
    dut.expect(re.compile(rb'50: -- 51 '), timeout=EXPECT_TIMEOUT)  # PCF85063A RTC
    dut.expect_exact(PROMPT, timeout=EXPECT_TIMEOUT)
    # Reconfigure with the default pins and a new frequency
    dut.write('i2cconfig --freq 400000')
    dut.expect_exact(PROMPT, timeout=EXPECT_TIMEOUT)
    dut.write('i2cdetect')
    dut.expect(re.compile(rb'50: -- 51 '), timeout=EXPECT_TIMEOUT)
    dut.expect_exact(PROMPT, timeout=EXPECT_TIMEOUT)

    def read_ram() -> int:
        dut.write(f'i2cget -c {PCF85063A_ADDR:#04x} -r {PCF85063A_RAM_BYTE_REG:#04x} -l 1')
        result = dut.expect(re.compile(rb'\r*\n(0x[0-9a-f]{2}) \r*\n'), timeout=EXPECT_TIMEOUT)
        dut.expect_exact(PROMPT, timeout=EXPECT_TIMEOUT)
        return int(result.group(1), 16)

    def write_ram(value: int) -> None:
        dut.write(f'i2cset -c {PCF85063A_ADDR:#04x} -r {PCF85063A_RAM_BYTE_REG:#04x} {value:#04x}')
        dut.expect_exact('Write OK', timeout=EXPECT_TIMEOUT)
        dut.expect_exact(PROMPT, timeout=EXPECT_TIMEOUT)

    # Restore the original byte even if a read-back assertion fails.
    original_value = read_ram()
    try:
        for value in (0x5A, 0xA5):
            write_ram(value)
            assert read_ram() == value
    finally:
        write_ram(original_value)
        assert read_ram() == original_value
