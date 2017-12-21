# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB), and the INTEL Visual Computing Lab.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# CORL experiment set.

from __future__ import print_function

import os
import datetime

from .benchmark import Benchmark
from .experiment import Experiment
from carla.sensor import Camera
from carla.settings import CarlaSettings


from .metrics import compute_summary



class CoRL2017(Benchmark):


    def get_all_statistics(self):

        summary = compute_summary(os.path.join(
            self._full_name, self._suffix_name),[3])

        return summary


    def plot_summary_train(self):


        self._plot_summary([1.0,3.0,6.0,8.0])

    def plot_summary_test(self):


        self._plot_summary([4.0,14.0])


    def _plot_summary(self,weathers):
        """
        We plot the summary of the testing for the set selected weathers.
        The test weathers are [4,14]

        """


        metrics_summary = compute_summary(os.path.join(
            self._full_name, self._suffix_name),[3])


        for metric,values in metrics_summary.items():

            print('Metric : ',metric)
            for weather,tasks in values.items():
                if weather in set(weathers):
                    print('  Weather: ',weather)
                    count =0
                    for t in tasks:

                        print('    Task ',count,' -> ',t)
                        count += 1


                    print('    AvG  -> ',float(sum(tasks))/float(len(tasks)))





    def _calculate_time_out(self,path_distance):
        """
        Function to return the timeout ( in miliseconds) that is calculated based on distance to goal.
        This is the same timeout as used on the CoRL paper.
        """

        return ((path_distance/100000.0)/10.0)*3600.0 + 10.0




    def _poses_town01(self):
        """
        Each matrix is a new task. We have all the four tasks

        """
        def _poses_straight():
            return [[36,40],[39,35],[110,114],[7,3],[0,4],
            [68,50],[61,59],[47,64],[147,90],[33,87],
            [26,19],[80,76],[45,49],[55,44],[29,107],
            [95,104],[84,34],[53,67],[22,17],[91,148],
            [20,107],[78,70],[95,102],[68,44],[45,69]]


        def _poses_one_curve():
            return [[138,17],[47,16],[26,9],[42,49],[140,124],
            [85,98],[65,133],[137,51],[76,66],[46,39],
            [40,60],[0,29],[4,129],[121,140],[2,129],
            [78,44],[68,85],[41,102],[95,70],[68,129],
            [84,69],[47,79],[110,15],[130,17],[0,17]]

        def _poses_navigation():
            return [[105,29],[27,130],[102,87],[132,27],[24,44],
            [96,26],[34,67],[28,1],[140,134],[105,9],
            [148,129],[65,18],[21,16],[147,97],[42,51],
            [30,41],[18,107],[69,45],[102,95],[18,145],
            [111,64],[79,45],[84,69],[73,31],[37,81]]


        return [ _poses_straight(),
                 _poses_one_curve(),
                 _poses_navigation(),
                 _poses_navigation()]



    def _poses_town02(self):


        def _poses_straight():
            return [[38, 34], [4, 2], [12, 10], [62, 55], [43, 47],
            [64, 66], [78, 76],[59,57],[61,18],[35,39],
            [12,8],[0,18],[75,68],[54,60],[45,49],
            [46,42],[53,46],[80,29],[65,63],[0,81],
            [54,63],[51,42],[16,19],[17,26],[77,68]]

        def _poses_one_curve():
            return [[37, 76], [8, 24], [60, 69], [38, 10], [21, 1],
            [58,71],[74,32],[44,0],[71,16],[14,24],
            [34,11],[43,14],[75,16],[80,21],[3,23],
            [75,59],[50,47],[11,19],[77,34],[79,25],
            [40,63],[58,76],[79,55],[16,61],[27,11]]

        def _poses_navigation():
            return [[19,66],[79,14],[19,57],[23,1],
            [53,76],[42,13],[31,71],[33,5],
            [54,30],[10,61],[66,3],[27,12],
            [79,19],[2,29],[16,14],[5,57],
            [70,73],[46,67],[57,50],[61,49],[21,12],
            [51,81],[77,68],[56,65],[43,54]]

        return [ _poses_straight(),
                 _poses_one_curve(),
                 _poses_navigation(),
                 _poses_navigation()
               ]


    def _build_experiments(self):
        """
        Creates the whole set of experiment objects,
        The experiments created depend on the selected Town.
        """

        # We set the camera
        # This single RGB camera is used on every experiment

        camera = Camera('CameraRGB')
        camera.set(CameraFOV=100)


        camera.set_image_size(800, 600)

        camera.set_position(200, 0, 140)
        camera.set_rotation(-15.0,0,0)

        weathers = [1,3,6,8,4,14]
        if self._city_name == 'Town01':
            poses_tasks = self._poses_town01()
            vehicles_tasks =[0,0,0,20]
            pedestrians_tasks = [0,0,0,50]
        else:
            poses_tasks = self._poses_town02()
            vehicles_tasks =[0,0,0,15]
            pedestrians_tasks = [0,0,0,50]



        experiments_vector = []


        for weather in weathers:


            for iteration in range(len(poses_tasks)):

                poses = poses_tasks[iteration]
                vehicles = vehicles_tasks[iteration]
                pedestrians = pedestrians_tasks[iteration]



                conditions = CarlaSettings()
                conditions.set(
                    SynchronousMode=True,
                    SendNonPlayerAgentsInfo=True,
                    NumberOfVehicles=vehicles,
                    NumberOfPedestrians=pedestrians,
                    WeatherId=weather,
                    SeedVehicles=123456789,
                    SeedPedestrians=123456789
                )
                # Add all the cameras that were set for this experiments

                conditions.add_sensor(camera)

                experiment = Experiment()
                experiment.set(
                    Conditions=conditions,
                    Poses=poses,
                    Id=iteration,
                    Repetitions=1
                    )
                experiments_vector.append(experiment)

        return experiments_vector

    def _get_details(self):

        # Function to get automatic information from the experiment for writing purposes
        return 'corl2017_' + self._city_name


    def _get_pose_and_task(self,line_on_file):
        """
        Returns the pose and task this experiment is, based on the line it was
        on the log file.
        """
        # We assume that the number of poses is constant
        return line_on_file/len(self._experiments),line_on_file%25

