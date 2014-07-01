/*
* File:   main.cpp
* Author: Saeid Emami
*
* Created on June 3, 2013, 5:58 PM
*/

//////////////////////////////////////////////////////////////////////////////////////////////////
// Available for use under the [MIT License](http://en.wikipedia.org/wiki/MIT_License)          //
// Writer: Saeid Emami                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////////

/* 
   Splits a layer of structured 2d mesh into two so that each layer is also structured. 

   The code is only intended to work with the profe macro divc2d.MAC.
*/

//#define GLFW_DLL

#include <iostream>
#include "Mesh.h"
#include <ctime>
#include "IO.h"
#include <sstream>
#include <stdlib.h>
#include <stdio.h>

//#define _OPENGL_AVAILABLE
#ifdef _OPENGL_AVAILABLE
#include <GLFW/glfw3.h>

static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

void secondwindow()
{
    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);
    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef((float) glfwGetTime() * 50.f, 0.f, 0.f, 1.f);
        glBegin(GL_TRIANGLES);
        glColor3f(1.f, 0.f, 0.f);
        glVertex3f(-0.6f, -0.4f, 0.f);
        glColor3f(0.f, 1.f, 0.f);
        glVertex3f(0.6f, -0.4f, 0.f);
        glColor3f(0.f, 0.f, 1.f);
        glVertex3f(0.f, 0.6f, 0.f);
        glEnd();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
}

#else

void secondwindow() {}

#endif

int main(int argc, char** argv) {
    TModel aModel;
    unsigned long long now = time(NULL);
    //std::thread t(secondwindow);

    starIO stario(aModel);
    ansysIO ansysio(aModel);
    abaqusIO abaqusio(aModel);

    std::string line1, line2;
    std::getline(std::cin, line1);
    std::getline(std::cin, line2);
    std::pair<size_t, size_t> inuputresult = stario.read(line1, line2);
    //std::cout << inuputresult.first << " vertices and " << inuputresult.second << " cells are read." << std::endl;

    std::getline(std::cin, line1);
    std::getline(std::cin, line2);
    size_t maximumv = strtoul(line1.c_str(), NULL, 10);
    size_t maximumc = strtoul(line2.c_str(), NULL, 10);
    if (maximumv >= aModel.global_vertices.size() || !aModel.global_vertices[maximumv])
        aModel.v(maximumv, 0, 0, 0, 0);
    if (maximumc >= aModel.global_cells.size() || !aModel.global_cells[maximumc])
        aModel.c(maximumc, std::vector<size_t>(), 0);
    //std::cout << "maximum vertex no.: " << maximumv << ", and maximum cell no.: " << maximumc << std::endl;

    std::getline(std::cin, line1);
    std::getline(std::cin, line2);
    size_t v1 = strtoul(line1.c_str(), NULL, 10);
    size_t v2 = strtoul(line2.c_str(), NULL, 10);
    //std::cout << "Dividing planar cells between vertices " << v1 << " and " << v2 << std::endl;
    TCoordMesh2D aCoordMesh(aModel);
    std::vector<std::vector<size_t>> result = aCoordMesh.divc(TEdge(v1, v2));
    std::set<size_t> newvset(result[0].cbegin(), result[0].cend());
    std::set<size_t> newcset(result[1].cbegin(), result[1].cend());
    std::set<size_t> oldcset(result[2].cbegin(), result[2].cend());

    std::getline(std::cin, line1);
    std::getline(std::cin, line2);
    std::pair<size_t, size_t> outputresult = stario.write(newvset, line1, newcset, line2);
    //std::cout << outputresult.first << " vertices and " << outputresult.second << " cells are written." << std::endl;

    std::getline(std::cin, line1);
    stario.ifilename = line1;
    //std::cout << oldcset.size() << " cells are to be deleted." << std::endl;
    stario.cset_ifile(oldcset);
    if (oldcset.size())
        stario.cdel_ifile();

    //TCoordMesh2D aCoordMesh(aModel);
    //aCoordMesh.updateEdges();
    //aCoordMesh.updateCoords();
    //std::cout << "size of coords: " << aCoordMesh.coords.size() << std::endl;
    /*for (size_t i = 0; i < aCoord.coords.size(); ++i) {
    std::cout << "coord no. " << i << ":" << std::endl;
    for (TCoord::const_iterator j = aCoord.coords[i].cbegin(); j != aCoord.coords[i].cend(); ++j) {
    std::cout << "[" << j->first.str() << ": ";
    for (std::set<size_t>::const_iterator k = j->second.cbegin(); k != j->second.cend(); ++k) {
    std::cout << *k << " ";
    }
    std::cout << "] "; 
    }
    std::cout <<std::endl;
    }*/
    //std::cout << "time elapsed: " << (time(NULL) - now) << std::endl;
    //std::cin.get();
    //t.join();
    return 0;
}
