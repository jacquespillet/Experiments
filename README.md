# Experiments

This repo contains some experiments I've been doing with openGL, trying to implement various computer graphics techniques.

Not all the experiments are successfull, some are still WIP, I may come back to them one day.


## Requirements : 
    Visual Studio (Tested only on Visual Studio 2019)


## Commands : 
```
### Clone the repo and checkout to the latest branch
git clone --recursive https://github.com/jacquespillet/Experiments.git
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

![VXGI](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/VXGI_1.png?raw=true)
![VXGI](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/VXGI_0.png?raw=true)

## [Global Illumination with Light Propagation Volumes]()

This is an incomplete implementation of Light Propagation Volumes. 

Here's a little write up about the implementation.

![LPV](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/LPV_2.png?raw=true)


## [Spherical Harmonics Viewer]()

A little tool to view spherical harmonics that I built while [reading](https://3dvar.com/Green2003Spherical.pdf) about them.

![SH](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/SH_0.PNG?raw=true)

## [GPU Bitonic Sort]()

An implementation of bitonic sort using compute shaders, with a little visualization.

![BitonicSort](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/Sort.gif?raw=true)


## [N-Bodies]()

Implementation of N-bodies simulation.

Here's a little write up about it

![NBodies]()
![NBodies]()
![NBodies]()


## [Fast Fourier Transform]()

Implementation of 1D and 2D fourier transform and inverse fast fourier transform.

![FFT](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/FFT_1d.PNG?raw=true)

This shows a signal (Top plot) that gets transformed in frequency space (Middle plot), and reconstructed using inverse fourier transform (Bottom Plot)

![FFT](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/FFT_2d.PNG?raw=true)

This shows an image input with its Fourier transform on the right, and the reconstructed image at the bottom.


## [Ocean FFT]()

This is an implementation of an ocean simulation with fourier transform, based on [this paper](https://people.computing.clemson.edu/~jtessen/reports/papers_files/coursenotes2004.pdf)

[Here]() is a little write up about the implementation

![OceanFFT](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/Ocean.png?raw=true)
![OceanFFT](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/OceanFFT_1.gif?raw=true)


## [Pic Flip Fluid]()

This is an implementation of Pic-Flip fluid simulation

[Here]()'s a little write up about the implementation


<table>
  <tr>
    <td><img src="https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/Fluid_0.gif?raw=true" alt="Image 1"></td>
    <td><img src="https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/Fluid_1.gif?raw=true" alt="Image 1"></td>
  </tr>
  <tr>
    <td><img src="https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/Fluid_2.gif?raw=true" alt="Image 1"></td>
    <td><img src="https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/Fluid_3.gif?raw=true" alt="Image 1"></td>
  </tr>
  <tr>
    <td><img src="https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/Fluid_4.gif?raw=true" alt="Image 1"></td>
    <td><img src="https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/Fluid_5.gif?raw=true" alt="Image 1"></td>
  </tr>
  <!-- Add more rows and images as needed -->
</table>

## [Decals](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/Decals.png?raw=true)

This is an implementation of deferred decals.

[Here]()'s a little write up about the implementation

![Decals](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/Decals.png?raw=true)


## [Path Tracer]()

This is a simple path tracer, based on Demofox's series on path tracing.

![PathTrace](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/PathTrace.png?raw=true)

## [Tiled Rendering]()

This is an implementation of Clustered and Forward+ rendering, with performance comparisons with forward and deferred.

![Tiled](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/Clustered_3.png?raw=true)

## [Screen Space Ambient Occlusion]()

Simple implementation of SSAO, based on [LearnOpenGL](https://learnopengl.com/)'s

![SSAO](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/SSAO_1.png?raw=true)

## [Screen Space Reflections]()

Simple implementation of screen space reflections, without fallback for out of screen reflections.

![SSR](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/SSR_1.png?raw=true)

## [Post Processing]()

Some simple post processing effects : 

Chromatic Aberation

![CA](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/PostProcessing_ChromaticAberation.png?raw=true)


Depth Of Field

![OIT](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/PostProcessing_DOFpng.png?raw=true)

CRT Effects

![OIT](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/PostProcessing_CRT.png?raw=true)

God Rays

![OIT](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/PostProcessing_GodRayspng.png?raw=true)

Vignette

![OIT](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/PostProcessing_Vignettepng.png?raw=true)

Sharpen Filter

![OIT](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/PostProcessing_Sharpenpng.png?raw=true)


## [Order Independant Transparency]()

Implementation of order independant transparency, based on [LearnOpenGL](https://learnopengl.com/)'s

Not sure if it's fully working...

![OIT](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/OIT.png?raw=true)

## [Cascaded Shadow Mapping]()

Implementation of cascaded shadow mapping, based on [LearnOpenGL](https://learnopengl.com/)'s



## [Shell Fur]()

Implementation of Shell Fur algorithm.

![ShellFur](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/ShellFure.png?raw=true)

## [Sparse Virtual Texturing]()

Implementation of Sparse Virtual Textures. It's almost working, just a problem to solve on the edges of the sub textures.

![SVT]()

## [Screen Space Directional Occlusion]()

Implementation of Screen Space Directional Occlusion.

I don't know if it's really working, getting odd results..

![SSDO](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/SSDO.png?raw=true)


## [Subdivision Surface]()

Implementation of Subdivision Surface algorithm.

<table>
  <tr>
    <td><img src="https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/Subdiv_0.png?raw=true" alt="Image 1"></td>
    <td><img src="https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/Subdiv_1.png?raw=true" alt="Image 1"></td>
    <td><img src="https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/Subdiv_2.png?raw=true" alt="Image 1"></td>
  </tr>
  <tr>
    <td><img src="https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/Subdiv_3.png?raw=true" alt="Image 1"></td>
    <td><img src="https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/Subdiv_4.png?raw=true" alt="Image 1"></td>
  </tr>
</table>

## [Sparse Voxel Octree]()

Implementation of sparse voxel octree construction on the GPU.

![RSM](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/SVO.png?raw=true)

## [Kuwahara Filtering]()

Implementation of Kuwahara Filter for images

<table>
  <tr>
    <td><img src="https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/Kuwahara.png?raw=true" alt="Image 1"></td>
    <td><img src="https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/Kuwahara_1.png?raw=true" alt="Image 1"></td>
    <td><img src="https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/Kuwahara_2.png?raw=true" alt="Image 1"></td>
  </tr>
</table>

## [Reflective Shadow Map]()

This is a failed attempt at implementing this technique. I'm not sure why it's not working, I may try to fix it later.


![RSM](https://github.com/jacquespillet/Experiments/blob/master/resources/Gallery/RSM.png?raw=true)