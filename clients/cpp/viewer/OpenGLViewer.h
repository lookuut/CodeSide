//
// Created by lookuut on 23.07.19.
//

#ifndef PAPER_OPENGLVIEWER_H
#define PAPER_OPENGLVIEWER_H

#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include "../models/Game.h"

static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

class OpenGLViewer{

    GLFWwindow* window;

    int width;
    int height;

public:

    OpenGLViewer(int width, int height): width(width), height(height) {
        glfwSetErrorCallback(error_callback);

        if (!glfwInit())
            exit(EXIT_FAILURE);

        window = glfwCreateWindow(width, height, "Paper IO", NULL, NULL);

        if (!window)
        {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        glfwMakeContextCurrent(window);
        glfwSetKeyCallback(window, key_callback);
    }


    void draw(const Game & world, const PointSet & points = PointSet()) {

        if (glfwWindowShouldClose(window)) {
            glfwDestroyWindow(window);
            glfwTerminate();
            exit(EXIT_SUCCESS);
        }

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, 1024, 1024, 0, 0, 1);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        int width = Config::width / 2;

        for (auto player : world.players) {
            auto color = player.second.territory.get_color();

            draw(player.second.territory.get_points_vector(), color, width);

            array<float, 3> line_color = { (color[0] > 0.5 ? color[0] / 2.f : color[0] * 2.f) , (color[1] > 0.5 ? color[1] / 2.f : color[1] * 2.f), (color[2] > 0.5 ? color[2] / 2.f : color[2] * 2.f)};
            draw(player.second.track.get_track(), line_color, width);
        }

        for (auto player : world.players) {
            auto color = player.second.territory.get_color();
            vector<Point> pos_vec = {player.second.pos};
            array<float, 3> pos_color = { (color[0] * 1.2f) , color[1] * 1.2f, color[2] * 1.2f};
            draw(pos_vec, pos_color, width);
        }

        if (!points.empty()) {
            draw(points, {1.f, 1.f,1.f}, width);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    void draw(const PointSet & points, const array<float, 3> & color, int width) {

        for (auto point : points) {
            glBegin(GL_QUADS);

            glColor3f(color[0], color[1], color[2]);
            glVertex2d(point.x - width, point.y - width);
            glVertex2d(point.x + width, point.y - width);
            glVertex2d(point.x + width, point.y + width);
            glVertex2d(point.x - width, point.y + width);

            glEnd();
        }
    }

    void draw(const vector<Point> & points, const array<float, 3> & color, int width) {

        for (auto point : points) {
            glBegin(GL_QUADS);

            glColor3f(color[0], color[1], color[2]);
            glVertex2d(point.x - width, point.y - width);
            glVertex2d(point.x + width, point.y - width);
            glVertex2d(point.x + width, point.y + width);
            glVertex2d(point.x - width, point.y + width);

            glEnd();
        }
    }

    void draw(const list<Point> & points, const array<float, 3> & color, int width) {

        for (auto point : points) {
            glBegin(GL_QUADS);

            glColor3f(color[0], color[1], color[2]);
            glVertex2d(point.x - width, point.y - width);
            glVertex2d(point.x + width, point.y - width);
            glVertex2d(point.x + width, point.y + width);
            glVertex2d(point.x - width, point.y + width);

            glEnd();
        }
    }
};

#endif //PAPER_OPENGLVIEWER_H
