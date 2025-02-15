@prism(type='fragment', name='Fast3D Fragment Shader', version='1.0.0', description='Ported shader to prism', author='Emill & Prism Team')

@{GLSL_VERSION}

@for(i in 0..2)
    @for(j in 0..2)
        @if(i == 9)
            // IF
        @elseif(j == 0)
            // ElseIf
        @else
            // Else
        @end
    @end
@end

@for(i in 0..2)
    @for(j in 0..2)
        @if(i == 9)
            // IF2
        @elseif(j == 0)
            // ElseIf2
        @else
            // Else2
        @end
    @end
@end