
struct tile 
{
    node[8];
}

struct node
{
    uint32_t childIndex;
    vec3 position;
    vec3 size;
}

buffer<tile> buffer = {-1}

rootNode =  {0}
lastIndex = 1;
levelStartIndices[12];

// Level 1
levelStartIndex = 0;
for all voxels
    if intersect with rootNode
        flag rootNode

oldLastIndex = lastIndex

if(rootNode is flagged)
    Allocate new node tiles :
        rootNode.childIndex = lastIndex;
        lastIndex +=8

for(int i=oldLastIndex; i<lastIndex; i++)
{
    buffers[rootNode.childIndex + i] = 0;
}

//level 2
levelStartIndex[1] = lastIndex;
for all voxels
    for nodes in rootNode.childIndex
        if(voxel intersects with node)
            flag node

oldLastIndex = lastIndex

for all nodes in rootNode.childIndex
    if node is flagged
        node.childIndex = lastIndex
        lastIndex += 8;

for nodes from oldLastIndex to lastIndex
    nodes[i].childIndex = 0;

//Level 3
levelStartIndex[2] = lastIndex;
flagNode(voxel, node){
    if(intersect(voxel, node))
    {
        if(node.childIndex==0) atomic_add(node.childIndex, 10000);
        else
        {
            for(int i=0; i<8; i++)
            {
                flagNode(voxel, buffer[node.childIndex + i])
            }
        }
    }
}

//Flag nodes that have to be subdivised
for all voxels 
    flagNode(voxel, rootNode)

//Allocate sub tiles
oldLastIndex = lastIndex;
i=lastIndex
while(true)
{
    if(buffer[i] is -1) break;

    if(buffer[i].childIndex == 1000000)
    {
        buffer[i].childIndex = lastIndex;
        lastIndex +=8;
    }
    i++;
} 

//Initialize new tiles
for(int i=oldLastIndex; i<lastIndex; i++)
{
    buffer[i] = 0;
}