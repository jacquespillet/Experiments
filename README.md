# Experiments

This repo contains some experiments I've been doing with openGL, trying to implement various computer graphics techniques.

Not all the experiments are successfull, some are still WIP, I may come back to them one day.


## Requirements : 
    Visual Studio (Tested only on Visual Studio 2019)


## Commands : 
```
### Clone the repo and checkout to the latest branch
git clone --recursive https://github.com/jacquespillet/gpupt_blog.git
cd Experiments
git checkout origin/master

### Generate the solution
mkdir build
cd build
cmake ../

### Build
cd ..
BuildDebug.bat / BuildRelease.bat

First build may take a while because it's going to build all the dependencies with the project.

```


## List of experiments

### [Voxel based global illumination]()

This is a simple implementation of voxel cone tracing.

Here's a little write up about that technique and its implementation.

![VXGI]()

## [Reflective Shadow Map]()

This is a failed attempt at implementing this technique. I'm not sure why it's not working, I may try to fix it later.


## [Global Illumination with Light Propagation Volumes]()

This is an incomplete implementation of Light Propagation Volumes. 

Here's a little write up about the implementation.

![LPV]()


## [Spherical Harmonics Viewer]()

A little tool to view spherical harmonics that I built while [reading](https://3dvar.com/Green2003Spherical.pdf) about them.

![SH]()

## [GPU Bitonic Sort]()

An implementation of bitonic sort using compute shaders, with a little visualization.

![BitonicSort]()


## [N-Bodies]()

Implementation of N-bodies simulation.

Here's a little write up about it

![NBodies]()
![NBodies]()
![NBodies]()


## [Fast Fourier Transform]()

Implementation of 1D and 2D fourier transform and inverse fast fourier transform.

![FFT]()
![FFT]()


## [Ocean FFT]()

This is an implementation of an ocean simulation with fourier transform, based on [this paper](https://people.computing.clemson.edu/~jtessen/reports/papers_files/coursenotes2004.pdf)

[Here]() is a little write up about the implementation

![OceanFFT]()
![OceanFFT]()


## [Pic Flip Fluid]()

This is an implementation of Pic-Flip fluid simulation

[Here]()'s a little write up about the implementation

![PicFlip]()
![PicFlip]()
![PicFlip]()
![PicFlip]()

## [Decals]()

This is an implementation of deferred decals.

[Here]()'s a little write up about the implementation

![PicFlip]()


## [Path Tracer]()

This is a simple path tracer, based on Demofox's series on path tracing.

![PathTrace]()

## [Tiled Rendering]()

This is an implementation of Clustered and Forward+ rendering, with performance comparisons with forward and deferred.

![Tiled]()
![Tiled]()
![Tiled]()
![Tiled]()


## [Screen Space Ambient Occlusion]()

## [Screen Space Reflections]()

## [Order Independant Transparency]()

## [Cascaded Shadow Mapping]()

## [Shell Fur]()

## [Sparse Virtual Texturing]()

## [Screen Space Directional Occlusion]()

## [Screen Space Subsurface Scattering]()

## [Post Processing]()

## [Subdivision Surface]()

## [Sparse Voxel Octree]()

## [Kuwahara Filtering]()