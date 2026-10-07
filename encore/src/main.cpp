/*
 * File: main.cpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-07
 * Purpose: Entrypoint for Cubot P50 Encore vendor AIDL service daemon.
 */

#include "EncoreService.hpp"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

using vendor::cubot::encore::EncoreService;

int main(int /*argc*/, char* argv[]) {
    android::base::InitLogging(argv, android::base::LogdLogger(android::base::SYSTEM));
    LOG(INFO) << "Starting Cubot P50 Encore Performance Daemon...";

    ABinderProcess_setThreadPoolMaxThreadCount(4);

    std::shared_ptr<EncoreService> service = ndk::SharedRefBase::make<EncoreService>();
    const std::string instance = std::string(EncoreService::descriptor) + "/default";

    binder_status_t status = AServiceManager_addService(service->asBinder().get(), instance.c_str());
    if (status != STATUS_OK) {
        LOG(FATAL) << "Failed to register " << instance << " with ServiceManager (error: " << status << ")";
        return EXIT_FAILURE;
    }

    LOG(INFO) << "Successfully registered " << instance << " with ServiceManager.";

    ABinderProcess_joinThreadPool();
    return EXIT_SUCCESS;
}
