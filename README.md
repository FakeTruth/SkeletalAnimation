# Skeletal Animation and Software Skinning
The aim of this project is to provide simple skeletal animation and software skinning (on the CPU) in C++.

This project can be used by developers who want to quickly add simple animations to their project. For commercial software this project may be... lacking... but can be used as a base to improve upon.

## Features
Feature       | Yes          | No     | Notes
------------- | :----------: | :----: | ----
Model Loading | V            |        | By using Assimp
Skeletal Animation | V       |        |
Vertex Animation |           | X      |
Rendering     |              | X      |
CPU Skinning  | V            |        |
GPU Skinning  |              | X      |
Skinned Normals |            | X      | To do
Multi-threaded  |            | X      | Will maybe add
Materials     |              | X      | To do
Textures      |              | X      | To do
Highly Optimized    |        | X      | 

## Dependencies
The SkeletalAnimation project is only dependent on [GLM](http://glm.g-truc.net/).

The AssimpConverter project is dependent on [Assimp](http://assimp.sourceforge.net/index.html) and SkeletalAnimation.

In order to compile the projects, both project files need to be modified to use the include files, and link with their respective dependencies.

## Usage
Make sure your project links with `SkeletalAnimation.lib`, or compile `SkeletalModel.cpp` into your project.

### Loading
If you do not have your own model loader you can use Assimp to load a model and use AssimpConverter to convert it to a `SkeletalModel` that can be rendered and animated:

```
void LoadModel()
{
	Assimp::Importer Importer;
	const aiScene* pScene = Importer.ReadFile("some_animated_model.fbx",
		aiProcess_LimitBoneWeights |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	AssimpConverter::Convert(pScene, g_AnimatedModel);
}
```

### Animating
After the `SkeletalModel` has been loaded it needs to be updated every frame to make it animate:
```
void Update(float a_Dt) // a_Dt is the elapsed time since last frame in seconds
{
  g_AnimatedModel.Update(a_Dt);
}
```

### Rendering
Here is some (very crude) example code to demonstrate how to render the animated model using OpenGL:
```
void Render()
{
	for (unsigned int i = 0; i < g_AnimatedModel.GetNumMeshes(); ++i)
	{
		const SA::sAnimatedMesh& AnimMesh = g_AnimatedModel.GetMesh(i);

		glBegin(GL_TRIANGLES);
		for (unsigned int i = 0; i < AnimMesh.NumIndices; ++i)
		{
			unsigned int Index = AnimMesh.pIndices[i];
			glm::vec3 n = AnimMesh.pNormals[Index];
			glm::vec3 v = AnimMesh.pTransformedVertices[Index];
			
			glColor4f(n.x, n.y, n.z, 1);
			glVertex3f(v.x, v.y, v.z);
		}
		glEnd();
	}
}
```

# References

http://ogldev.atspace.co.uk/www/tutorial38/tutorial38.html
