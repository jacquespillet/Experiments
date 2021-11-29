#version 440 core
out vec4 fragColor;
  
in vec2 fragUv;

uniform sampler2D textureImage;
uniform sampler2D K0;

uniform float q = 8;
uniform int radius;
uniform int currentMode;
const float PI = 3.14159265358979323846;
vec3 KuwaharaFilter()
{
    vec3 result = texture(textureImage, fragUv).rgb;
    vec2 textureSize = textureSize(textureImage, 0);
    
    float n = float((radius + 1) * ( radius + 1));
    
    //Med and squared
    vec3 m[4];
    vec3 s[4];
    for (int k = 0; k < 4; ++ k) { 
        m[k] = vec3(0.0);
        s[k] = vec3(0.0);
    }

    //Create the 4 sub squares
    struct Window { int x1, y1, x2, y2; }; 
    Window W[4] = Window[4](
        Window( - radius , - radius , 0, 0 ), 
        Window( 0, - radius , radius , 0 ), 
        Window( 0, 0, radius , radius ),
        Window( - radius , 0, 0, radius )
    );
    
    //For each sub quare
    for (int k = 0; k < 4; ++ k) { 

        //Go through all the pixels inside the region
        for (int j = W[k].y1; j <= W[k].y2; ++ j) { 
            for (int i = W[k].x1; i <= W[k].x2; ++ i) {
                //Compute the mean and the variance of that region 
                vec3 c = texture(textureImage, fragUv + vec2(i,j) / textureSize).rgb;
                m[k] += c;
                s[k] += c * c;
            }
        }
    }

    float minVariance = 1e+2;

    //For each region
    for (int k = 0; k < 4; ++ k) { 
        //Calculate the variance
        m[k] /= n;
        s[k] = abs(s[k] / n - m[k] * m[k]);

        //Calculate the sum of the channel variances
        float sigma2 = s[k].r + s[k].g + s[k].b;

        //Store the mimimum variance. Set the result to be the average of that region
        if (sigma2 < minVariance) { 
            minVariance = sigma2;
            result = m[k];
        }
    }    
    return result;
}

vec3 GeneralizedKuwaharaFilter()
{
    int N = 8;

    vec3 result = texture(textureImage, fragUv).rgb;
    vec2 textureSize = textureSize(textureImage, 0);
    
    //Initialize the average and variance
    vec4 m[8];
    vec3 s[8];
    for (int k = 0; k < N; ++ k) { 
        m[k] = vec4(0.0);
        s[k] = vec3(0.0);
    }

    //2D rotation matrix that rotates 1 out of N steps anti clockwise.
    float piN = 2.0f * PI / float(N);
    mat2 X = mat2(cos(piN), sin(piN), -sin(piN), cos(piN));

    
    for ( int j = - radius; j <= radius; ++ j ) { 
        for ( int i = - radius; i <= radius; ++ i ) { 
            //normalize from [-radius, radius] to [-0.5, 0.5]
            vec2 v = 0.5 * vec2(i,j) / float(radius);

            //If the length of this is less than 0.5 (squared = 0.25)
            if (dot(v,v) <= 0.25) {
                //Sample the image at this position
                vec3 c = texture(textureImage, fragUv + vec2(i,j) / textureSize).rgb;
                
                //For each sector, calculate the weight of this sample
                for (int k = 0; k < N; ++ k) { 
                    //Sample the weight at this pixel position
                    float w = texture(K0, vec2(0.5, 0.5) + v).x;
                    
                    //Add to the mean and variance accumulators
                    m[k] += vec4(c * w, w); //store the weighted color, and the sum of weights
                    s[k] += c * c * w; //Store the square of the color weighted

                    //Rotate the vector
                    v *= X;
                }
            }
        }
    }

    //Normalization
    vec4 o = vec4(0.0);

    //For each sector
    for (int k = 0; k < N; ++ k) { 
        //Normalize the color mean
        m[k].rgb /= m[k].w;
        
        //normalize the color variance
        s[k] = abs(s[k] / m[k].w - m[k].rgb * m[k].rgb);
        
        //Variance value
        float sigma2 = s[k].r + s[k].g + s[k].b;
        
        //calculate output
        float w = 1.0 / (1.0 + pow(255.0 * sigma2 , 0.5 * q));
        o += vec4(m[k].rgb * w, w);
    }
    return o.rgb / o.w;
}

// uniform vec2
void main()
{
    vec3 result;
    if(currentMode==0) result = texture(textureImage, fragUv).rgb;
    else if(currentMode==1)  result = KuwaharaFilter();    
    else if(currentMode==2)  result = GeneralizedKuwaharaFilter();    
    result = pow(result, vec3(1/2.2));
    fragColor = vec4(result, 1.0f) ;
}
