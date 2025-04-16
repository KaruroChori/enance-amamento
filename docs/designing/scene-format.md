XML files to describe SDF. Implementation split in its own library. It will be probably based on part of the code from vs.

```xml
<xml>
    <meta>
        <author></author>
        <title></title>
    </meta>
    <world>
        <materials>
            <material name="#arc" uid="0" type="color">
                <albedo value="#ff3022"/>
            </material>
            <material uid="1" type="styleblit">
                <texture src="..."/>
                <normals src="..."/>
            </material>
        </materials>
        <lineart>
        </lineart>
    </world>
    <camera type="projection"></camera>
    <scene>
        <group gid="12">
            <sphere radius="5.0" material="#arc" name="#call-me-back" uid="*"/>
        </group>
    </scene>
    <expressions>
        <expression name="$ex1">code in here</expression>
    </expressions>
</xml>
```

Let's consider the basics first. Basically the fragment inside scene, the rest is built on top of that. Also, let's ignore mnemonics for material and name for now.

```xml
<entity ui-name="" pos="a|b|c" pos.0="" pos.1="" pos.2="" pos.x="" pos.r="" pos.u=""> <!--Automatically handle alias names, first match first taken from the fixed search list-->
    <attrs gid="12" uid="*" idx="5" weak="true"/>
</entity>
```
