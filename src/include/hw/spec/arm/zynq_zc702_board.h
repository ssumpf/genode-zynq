/*
 * \brief   Zynq zc702 specific board definitions
 * \author  Johannes Schlatow
 * \date    2021-08-18
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__INCLUDE__HW__SPEC__ARM__ZYNQ_ZC702_BOARD_H_
#define _SRC__INCLUDE__HW__SPEC__ARM__ZYNQ_ZC702_BOARD_H_

#include <hw/spec/arm/zynq_zc702.h>
#include <drivers/uart/xilinx.h>
#include <hw/spec/arm/cortex_a9.h>
#include <hw/spec/arm/boot_info.h>

namespace Hw::Zynq_zc702_board {

	using namespace Zynq_zc702;
	using Cpu_mmio = Hw::Cortex_a9_mmio<CORTEX_A9_PRIVATE_MEM_BASE>;
	using Serial   = Genode::Xilinx_uart;

	enum {
		UART_BASE  = UART_1_MMIO_BASE,
	};
}

#endif /* _SRC__INCLUDE__HW__SPEC__ARM__ZYNQ_ZC702_BOARD_H_ */
