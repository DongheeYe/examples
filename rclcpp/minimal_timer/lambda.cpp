// Copyright 2016 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <chrono>
#include <memory>
#include <glib.h>


#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

/* This example creates a subclass of Node and uses a fancy C++11 lambda
 * function to shorten the timer syntax, at the expense of making the
 * code somewhat more difficult to understand at first glance if you are
 * unaccustomed to C++11 lambda expressions. */

class MinimalTimer : public rclcpp::Node
{
public:
  MinimalTimer()
  : Node("minimal_timer")
  {
    auto timer_callback = [this]() -> void {
      RCLCPP_INFO(this->get_logger(), "Timer cancel");
      timer_->cancel();
    };
    timer_ = create_wall_timer(500ms, timer_callback);
  }

//private:
  rclcpp::TimerBase::SharedPtr timer_;
};

GMainLoop *mainloop;
std::shared_ptr<rclcpp::executors::SingleThreadedExecutor> exec;
std::shared_ptr<MinimalTimer> node;


gboolean TimerExpired(gpointer data) {
   if (node->timer_->is_canceled()){
     RCLCPP_INFO(node->get_logger(), "Timer was cancelled and reset");
     node->timer_->reset();
   }else {
     RCLCPP_INFO(node->get_logger(), "Timer was not cancelled");
   }
   return true;
}

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  exec = std::make_shared<rclcpp::executors::SingleThreadedExecutor>();
  node = std::make_shared<MinimalTimer>();
  exec->add_node(node);

  mainloop = g_main_loop_new(nullptr, false);

  g_timeout_add(1000, TimerExpired, nullptr);

  g_thread_new(nullptr,
        [](gpointer data) -> gpointer {
          exec->spin();
          return nullptr;
        },
        static_cast<void *>(node.get()));

  g_main_loop_run(mainloop);

  rclcpp::shutdown();
  return 0;
}
