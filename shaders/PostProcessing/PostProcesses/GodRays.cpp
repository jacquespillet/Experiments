float4 main(float2 texCoord : TEXCOORD0) : COLOR0 {   
    // Calculate vector from pixel to light source in screen space.    
    half2 deltaTexCoord = (texCoord - ScreenLightPos.xy);   
    // Divide by number of samples and scale by control factor.   
    deltaTexCoord *= 1.0f / NUM_SAMPLES * Density;   
    
    // Store initial sample.    
    half3 color = tex2D(frameSampler, texCoord);   
    // Set up illumination decay factor.    
    half illuminationDecay = 1.0f;   
    
    // Evaluate summation from Equation 3 NUM_SAMPLES iterations.    
    for (int i = 0; i < NUM_SAMPLES; i++)   {     
        // Step sample location along ray.     
        texCoord -= deltaTexCoord;     
        // Retrieve sample at new location.    
        half3 sample = tex2D(frameSampler, texCoord);     
        // Apply sample attenuation scale/decay factors.     
        sample *= illuminationDecay * Weight;     
        // Accumulate combined color.     
        color += sample;     
        // Update exponential decay factor.     
        illuminationDecay *= Decay;   
    }   
    
    // Output final color with a further scale control factor.    
    return float4( color * Exposure, 1); 
} 