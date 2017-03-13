/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __MCE_PRIVATE_H__
#define __MCE_PRIVATE_H__

#include <mmio.h>
#include <tegra_def.h>

/*******************************************************************************
 * Macros to prepare CSTATE info request
 ******************************************************************************/
/* Description of the parameters for UPDATE_CSTATE_INFO request */
#define CLUSTER_CSTATE_MASK			0x7ULL
#define CLUSTER_CSTATE_SHIFT			0U
#define CLUSTER_CSTATE_UPDATE_BIT		(1ULL << 7)
#define CCPLEX_CSTATE_MASK			0x3ULL
#define CCPLEX_CSTATE_SHIFT			8ULL
#define CCPLEX_CSTATE_UPDATE_BIT		(1ULL << 15)
#define SYSTEM_CSTATE_MASK			0xFULL
#define SYSTEM_CSTATE_SHIFT			16ULL
#define SYSTEM_CSTATE_FORCE_UPDATE_SHIFT	22ULL
#define SYSTEM_CSTATE_FORCE_UPDATE_BIT		(1ULL << 22)
#define SYSTEM_CSTATE_UPDATE_BIT		(1ULL << 23)
#define CSTATE_WAKE_MASK_UPDATE_BIT		(1ULL << 31)
#define CSTATE_WAKE_MASK_SHIFT			32ULL
#define CSTATE_WAKE_MASK_CLEAR			0xFFFFFFFFU

/*******************************************************************************
 * Auto-CC3 control macros
 ******************************************************************************/
#define MCE_AUTO_CC3_FREQ_MASK			0x1FFU
#define MCE_AUTO_CC3_FREQ_SHIFT			0U
#define MCE_AUTO_CC3_VTG_MASK			0x7FU
#define MCE_AUTO_CC3_VTG_SHIFT			16U
#define MCE_AUTO_CC3_ENABLE_BIT			(1U << 31)

/*******************************************************************************
 * Macros for the 'IS_SC7_ALLOWED' command
 ******************************************************************************/
#define MCE_SC7_ALLOWED_MASK			0x7U
#define MCE_SC7_WAKE_TIME_SHIFT			32U

/*******************************************************************************
 * Macros for 'read/write ctats' commands
 ******************************************************************************/
#define MCE_CSTATE_STATS_TYPE_SHIFT		32ULL
#define MCE_CSTATE_WRITE_DATA_LO_MASK		0xFU

/*******************************************************************************
 * Macros for 'update crossover threshold' command
 ******************************************************************************/
#define MCE_CROSSOVER_THRESHOLD_TIME_SHIFT	32U

/*******************************************************************************
 * MCA command struct
 ******************************************************************************/
typedef union mca_cmd {
	struct command {
		uint8_t cmd;
		uint8_t idx;
		uint8_t subidx;
	} command;
	struct input {
		uint32_t low;
		uint32_t high;
	} input;
	uint64_t data;
} mca_cmd_t;

/*******************************************************************************
 * MCA argument struct
 ******************************************************************************/
typedef union mca_arg {
	struct err {
		uint32_t error:8;
		uint32_t unused:24;
		uint32_t unused2:24;
		uint32_t finish:8;
	} err;
	struct arg {
		uint32_t low;
		uint32_t high;
	} arg;
	uint64_t data;
} mca_arg_t;

/*******************************************************************************
 * Uncore PERFMON ARI struct
 ******************************************************************************/
typedef union uncore_perfmon_req {
	struct perfmon_command {
		/*
		 * Commands: 0 = READ, 1 = WRITE
		 */
		uint32_t cmd:8;
		/*
		 * The unit group: L2=0, L3=1, ROC=2, MC=3, IOB=4
		 */
		uint32_t grp:4;
		/*
		 * Unit selector: Selects the unit instance, with 0 = Unit
		 * = (number of units in group) - 1.
		 */
		uint32_t unit:4;
		/*
		 * Selects the uncore perfmon register to access
		 */
		uint32_t reg:8;
		/*
		 * Counter number. Selects which counter to use for
		 * registers NV_PMEVCNTR and NV_PMEVTYPER.
		 */
		uint32_t counter:8;
	} perfmon_command;
	struct perfmon_status {
		/*
		 * Resulting command status
		 */
		uint32_t val:8;
		uint32_t unused:24;
	} perfmon_status;
	uint64_t data;
} uncore_perfmon_req_t;

#define UNCORE_PERFMON_CMD_READ			0U
#define UNCORE_PERFMON_CMD_WRITE		1U

#define UNCORE_PERFMON_CMD_MASK			0xFFU
#define UNCORE_PERFMON_UNIT_GRP_MASK		0xFU
#define UNCORE_PERFMON_SELECTOR_MASK		0xFU
#define UNCORE_PERFMON_REG_MASK			0xFFU
#define UNCORE_PERFMON_CTR_MASK			0xFFU
#define UNCORE_PERFMON_RESP_STATUS_MASK		0xFFU

/*******************************************************************************
 * Structure populated by arch specific code to export routines which perform
 * common low level MCE functions
 ******************************************************************************/
typedef struct arch_mce_ops {
	/*
	 * This ARI request sets up the MCE to start execution on assertion
	 * of STANDBYWFI, update the core power state and expected wake time,
	 * then determine the proper power state to enter.
	 */
	int (*enter_cstate)(uint32_t ari_base, uint32_t state,
			    uint32_t wake_time);
	/*
	 * This ARI request allows updating of the CLUSTER_CSTATE,
	 * CCPLEX_CSTATE, and SYSTEM_CSTATE register values.
	 */
	int (*update_cstate_info)(uint32_t ari_base,
				  uint32_t cluster,
				  uint32_t ccplex,
				  uint32_t system,
				  uint8_t sys_state_force,
				  uint32_t wake_mask,
				  uint8_t update_wake_mask);
	/*
	 * This ARI request allows updating of power state crossover
	 * threshold times. An index value specifies which crossover
	 * state is being updated.
	 */
	int (*update_crossover_time)(uint32_t ari_base,
				     uint32_t type,
				     uint32_t time);
	/*
	 * This ARI request allows read access to statistical information
	 * related to power states.
	 */
	uint64_t (*read_cstate_stats)(uint32_t ari_base,
				     uint32_t state);
	/*
	 * This ARI request allows write access to statistical information
	 * related to power states.
	 */
	int (*write_cstate_stats)(uint32_t ari_base,
				  uint32_t state,
				  uint32_t stats);
	/*
	 * This ARI request allows the CPU to understand the features
	 * supported by the MCE firmware.
	 */
	uint64_t (*call_enum_misc)(uint32_t ari_base, uint32_t cmd,
				   uint32_t data);
	/*
	 * This ARI request allows querying the CCPLEX to determine if
	 * the CCx state is allowed given a target core C-state and wake
	 * time. If the CCx state is allowed, the response indicates CCx
	 * must be entered. If the CCx state is not allowed, the response
	 * indicates CC6/CC7 can't be entered
	 */
	int (*is_ccx_allowed)(uint32_t ari_base, uint32_t state,
			      uint32_t wake_time);
	/*
	 * This ARI request allows querying the CCPLEX to determine if
	 * the SC7 state is allowed given a target core C-state and wake
	 * time. If the SC7 state is allowed, all cores but the associated
	 * core are offlined (WAKE_EVENTS are set to 0) and the response
	 * indicates SC7 must be entered. If the SC7 state is not allowed,
	 * the response indicates SC7 can't be entered
	 */
	int (*is_sc7_allowed)(uint32_t ari_base, uint32_t state,
			      uint32_t wake_time);
	/*
	 * This ARI request allows a core to bring another offlined core
	 * back online to the C0 state. Note that a core is offlined by
	 * entering a C-state where the WAKE_MASK is all 0.
	 */
	int (*online_core)(uint32_t ari_base, uint32_t cpuid);
	/*
	 * This ARI request allows the CPU to enable/disable Auto-CC3 idle
	 * state.
	 */
	int (*cc3_ctrl)(uint32_t ari_base,
			uint32_t freq,
			uint32_t volt,
			uint8_t enable);
	/*
	 * This ARI request allows updating the reset vector register for
	 * D15 and A57 CPUs.
	 */
	int (*update_reset_vector)(uint32_t ari_base);
	/*
	 * This ARI request instructs the ROC to flush A57 data caches in
	 * order to maintain coherency with the Denver cluster.
	 */
	int (*roc_flush_cache)(uint32_t ari_base);
	/*
	 * This ARI request instructs the ROC to flush A57 data caches along
	 * with the caches covering ARM code in order to maintain coherency
	 * with the Denver cluster.
	 */
	int (*roc_flush_cache_trbits)(uint32_t ari_base);
	/*
	 * This ARI request instructs the ROC to clean A57 data caches along
	 * with the caches covering ARM code in order to maintain coherency
	 * with the Denver cluster.
	 */
	int (*roc_clean_cache)(uint32_t ari_base);
	/*
	 * This ARI request reads/writes the Machine Check Arch. (MCA)
	 * registers.
	 */
	uint64_t (*read_write_mca)(uint32_t ari_base,
			      mca_cmd_t cmd,
			      uint64_t *data);
	/*
	 * Some MC GSC (General Security Carveout) register values are
	 * expected to be changed by TrustZone secure ARM code after boot.
	 * Since there is no hardware mechanism for the CCPLEX to know
	 * that an MC GSC register has changed to allow it to update its
	 * own internal GSC register, there needs to be a mechanism that
	 * can be used by ARM code to cause the CCPLEX to update its GSC
	 * register value. This ARI request allows updating the GSC register
	 * value for a certain carveout in the CCPLEX.
	 */
	int (*update_ccplex_gsc)(uint32_t ari_base, uint32_t gsc_idx);
	/*
	 * This ARI request instructs the CCPLEX to either shutdown or
	 * reset the entire system
	 */
	void (*enter_ccplex_state)(uint32_t ari_base, uint32_t state_idx);
	/*
	 * This ARI request reads/writes data from/to Uncore PERFMON
	 * registers
	 */
	int (*read_write_uncore_perfmon)(uint32_t ari_base,
			uncore_perfmon_req_t req, uint64_t *data);
	/*
	 * This ARI implements ARI_MISC_CCPLEX commands. This can be
	 * used to enable/disable coresight clock gating.
	 */
	void (*misc_ccplex)(uint32_t ari_base, uint32_t index,
			uint32_t value);
} arch_mce_ops_t;

/* declarations for ARI/NVG handler functions */
int ari_enter_cstate(uint32_t ari_base, uint32_t state, uint32_t wake_time);
int ari_update_cstate_info(uint32_t ari_base, uint32_t cluster, uint32_t ccplex,
	uint32_t system, uint8_t sys_state_force, uint32_t wake_mask,
	uint8_t update_wake_mask);
int ari_update_crossover_time(uint32_t ari_base, uint32_t type, uint32_t time);
uint64_t ari_read_cstate_stats(uint32_t ari_base, uint32_t state);
int ari_write_cstate_stats(uint32_t ari_base, uint32_t state, uint32_t stats);
uint64_t ari_enumeration_misc(uint32_t ari_base, uint32_t cmd, uint32_t data);
int ari_is_ccx_allowed(uint32_t ari_base, uint32_t state, uint32_t wake_time);
int ari_is_sc7_allowed(uint32_t ari_base, uint32_t state, uint32_t wake_time);
int ari_online_core(uint32_t ari_base, uint32_t core);
int ari_cc3_ctrl(uint32_t ari_base, uint32_t freq, uint32_t volt, uint8_t enable);
int ari_reset_vector_update(uint32_t ari_base);
int ari_roc_flush_cache_trbits(uint32_t ari_base);
int ari_roc_flush_cache(uint32_t ari_base);
int ari_roc_clean_cache(uint32_t ari_base);
uint64_t ari_read_write_mca(uint32_t ari_base, mca_cmd_t cmd, uint64_t *data);
int ari_update_ccplex_gsc(uint32_t ari_base, uint32_t gsc_idx);
void ari_enter_ccplex_state(uint32_t ari_base, uint32_t state_idx);
int ari_read_write_uncore_perfmon(uint32_t ari_base,
		uncore_perfmon_req_t req, uint64_t *data);
void ari_misc_ccplex(uint32_t ari_base, uint32_t index, uint32_t value);

int nvg_enter_cstate(uint32_t ari_base, uint32_t state, uint32_t wake_time);
int nvg_update_cstate_info(uint32_t ari_base, uint32_t cluster, uint32_t ccplex,
		uint32_t system, uint8_t sys_state_force, uint32_t wake_mask,
		uint8_t update_wake_mask);
int nvg_update_crossover_time(uint32_t ari_base, uint32_t type, uint32_t time);
uint64_t nvg_read_cstate_stats(uint32_t ari_base, uint32_t state);
int nvg_write_cstate_stats(uint32_t ari_base, uint32_t state, uint32_t val);
int nvg_is_ccx_allowed(uint32_t ari_base, uint32_t state, uint32_t wake_time);
int nvg_is_sc7_allowed(uint32_t ari_base, uint32_t state, uint32_t wake_time);
int nvg_online_core(uint32_t ari_base, uint32_t core);
int nvg_cc3_ctrl(uint32_t ari_base, uint32_t freq, uint32_t volt, uint8_t enable);

#endif /* __MCE_PRIVATE_H__ */
