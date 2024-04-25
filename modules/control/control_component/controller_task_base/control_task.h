/******************************************************************************
 * Copyright 2017 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

/**
 * @file
 * @brief Defines the Controller base class.
 */

#pragma once

#include <memory>
#include <string>

#include <cxxabi.h>

#include "modules/common_msgs/control_msgs/control_cmd.pb.h"
#include "modules/common_msgs/localization_msgs/localization.pb.h"
#include "modules/common_msgs/planning_msgs/planning.pb.h"
#include "modules/control/control_component/proto/calibration_table.pb.h"

#include "cyber/common/file.h"
#include "cyber/plugin_manager/plugin_manager.h"
#include "modules/common/status/status.h"
#include "modules/control/control_component/common/control_gflags.h"
#include "modules/control/control_component/controller_task_base/common/dependency_injector.h"

/**
 * @namespace apollo::control
 * @brief apollo::control
 */
namespace apollo {
namespace control {

/**
 * @class Controller
 *
 * @brief base class for all controllers.
 */
class ControlTask {
 public:
  /**
   * @brief constructor
   */
  ControlTask() = default;

  /**
   * @brief destructor
   */
  virtual ~ControlTask() = default;

  /**
   * @brief initialize Controller
   * @param control_conf control configurations
   * @return Status initialization status
   */
  virtual common::Status Init(std::shared_ptr<DependencyInjector> injector) = 0;

  /**
   * @brief compute control command based on current vehicle status
   *        and target trajectory
   * @param localization vehicle location
   * @param chassis vehicle status e.g., speed, acceleration
   * @param trajectory trajectory generated by planning
   * @param cmd control command
   * @return Status computation status
   */
  virtual common::Status ComputeControlCommand(
      const localization::LocalizationEstimate *localization,
      const canbus::Chassis *chassis, const planning::ADCTrajectory *trajectory,
      control::ControlCommand *cmd) = 0;

  /**
   * @brief reset Controller
   * @return Status reset status
   */
  virtual common::Status Reset() = 0;

  /**
   * @brief controller name
   * @return string controller name in string
   */
  virtual std::string Name() const = 0;

  /**
   * @brief stop controller
   */
  virtual void Stop() = 0;

 protected:
  template <typename T>
  bool LoadConfig(T *config);

  bool LoadCalibrationTable(calibration_table *calibration_table_conf) {
    std::string calibration_table_path = FLAGS_calibration_table_file;

    if (!apollo::cyber::common::GetProtoFromFile(calibration_table_path,
                                                 calibration_table_conf)) {
      AERROR << "Load calibration table failed!";
      return false;
    }
    AINFO << "Load the calibraiton table file successfully, file path: "
          << calibration_table_path;
    return true;
  }
};

template <typename T>
bool ControlTask::LoadConfig(T *config) {
  int status;
  std::string class_name =
      abi::__cxa_demangle(typeid(*this).name(), 0, 0, &status);
  // Generate the default task config path from PluginManager.
  std::string config_path_ =
      apollo::cyber::plugin_manager::PluginManager::Instance()
          ->GetPluginConfPath<ControlTask>(class_name,
                                           "conf/controller_conf.pb.txt");

  if (!apollo::cyber::common::GetProtoFromFile(config_path_, config)) {
    AERROR << "Load config of " << class_name << " failed!";
    return false;
  }
  AINFO << "Load the [" << class_name
        << "] config file successfully, file path: " << config_path_;
  return true;
}

}  // namespace control
}  // namespace apollo