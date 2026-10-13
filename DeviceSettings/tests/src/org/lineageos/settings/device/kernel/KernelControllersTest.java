/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * Author: Onogwu Franklin Kelechi (kelexine) <https://github.com/kelexine>
 * Date: 2026-09-18
 * Purpose: Unit tests for kernel tuning controllers.
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.device.kernel;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import org.junit.Test;

import java.util.List;

public class KernelControllersTest {

    @Test
    public void cpuidle_defaultGovernor_isTeo() {
        assertEquals("teo", CpuidleController.DEFAULT_GOVERNOR);
    }

    @Test
    public void cpuidle_availableGovernors_notNull() {
        List<String> govs = CpuidleController.getAvailableGovernors();
        assertNotNull(govs);
        assertTrue(govs.contains("teo"));
    }

    @Test
    public void ioScheduler_defaultScheduler_isBfq() {
        assertEquals("bfq", IoSchedulerController.DEFAULT_SCHEDULER);
    }

    @Test
    public void ioScheduler_defaultReadahead_is512() {
        assertEquals("512", IoSchedulerController.DEFAULT_READ_AHEAD_KB);
    }

    @Test
    public void tcpCongestion_defaultAlgorithm_isBbr() {
        assertEquals("bbr", TcpCongestionController.DEFAULT_CONGESTION);
    }

    @Test
    public void compaction_defaultProactiveness_is20() {
        assertEquals("20", CompactionController.DEFAULT_PROACTIVENESS);
    }

    @Test
    public void setGovernor_withNullOrEmpty_returnsFalse() {
        assertFalse(CpuidleController.setGovernor(null));
        assertFalse(CpuidleController.setGovernor(""));
    }

    @Test
    public void setScheduler_withNullOrEmpty_returnsFalse() {
        assertFalse(IoSchedulerController.setScheduler(null));
        assertFalse(IoSchedulerController.setScheduler(""));
    }

    @Test
    public void setCongestion_withNullOrEmpty_returnsFalse() {
        assertFalse(TcpCongestionController.setCongestion(null));
        assertFalse(TcpCongestionController.setCongestion(""));
    }

    @Test
    public void setProactiveness_withNullOrEmpty_returnsFalse() {
        assertFalse(CompactionController.setProactiveness(null));
        assertFalse(CompactionController.setProactiveness(""));
    }
}
