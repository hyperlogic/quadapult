//
//  Shader.fsh
//  quadapult
//
//  Created by Anthony Thibault on 1/28/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}
