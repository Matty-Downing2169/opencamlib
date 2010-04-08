import ocl
import camvtk
import time

def printNodes(t):
    nodes = t.get_nodes()
    for n in nodes:
        print n.str()

def drawTree(myscreen,t,color=camvtk.red,opacity=0.2, offset=(0,0,0)):
    nodes = t.get_nodes()
    black=0
    for n in nodes:
        cen = n.point()
        #print "cen=",cen.str()
        scale = n.get_scale()
        #print "col=", n.color
        
        if n.color == 0:
            #print "found white node!"
            #color = camvtk.red
            cube = camvtk.Cube(center=(cen.x+offset[0], cen.y+offset[1], cen.z+offset[2]), length= scale, color=color)
            cube.SetOpacity(opacity)
            myscreen.addActor( cube )
            black = black+1
        if n.color == 1:
            #print "found white node!"6
            color = camvtk.blue
            cube = camvtk.Cube(center=(cen.x, cen.y, cen.z), length= scale, color=color)
            cube.SetOpacity(opacity)
            myscreen.addActor( cube )
            #black = black+1
    #print black," black nodes"
    """
    for m in xrange(0,9):
        cen = n.corner(m)
        sph = camvtk.Sphere( center=(cen.x, cen.y, cen.z), radius=0.5, color=camvtk.green)
        myscreen.addActor(sph)
    """
        #myscreen.render()
        #raw_input("Press Enter to terminate")
        
f=ocl.Ocode()
f.set_depth(7)


myscreen = camvtk.VTKScreen()   
myscreen.camera.SetPosition(80, 52, 20)
myscreen.camera.SetFocalPoint(0,0, 0)   

xar = camvtk.Arrow(color=camvtk.red, rotXYZ=(0,0,0))
myscreen.addActor(xar)
yar = camvtk.Arrow(color=camvtk.green, rotXYZ=(0,0,90))
myscreen.addActor(yar)
zar = camvtk.Arrow(color=camvtk.blue, rotXYZ=(0,-90,0))
myscreen.addActor(zar) 


t = ocl.LinOCT()
print " created: ",t.str()
#t.append(o2)
t.init(3)
t2 = ocl.LinOCT()
t2.init(3)

print " after init():", t.str()
#printNodes(t)
print "t.size=", t.size()

svol = ocl.SphereOCTVolume()
svol.radius=3
svol.center = ocl.Point(4,2,3)

cube1 = ocl.CubeOCTVolume()
cube1.side=6
cube1.center = ocl.Point(0,0,0)

cube2 = ocl.CubeOCTVolume()
cube2.center = ocl.Point(1,2,0)
cube2.side = 30








#drawTree(myscreen,t,opacity=0.1, color=camvtk.grey)

print "t build()"    
t.build(svol)
print " t after build() ", t.size()
t.condense()
print " t after condense() ", t.size()

print "t2 build()" 
t2.build(cube1)
print " t2 after build() ", t2.size()
t2.condense()
print " t2 after condense() ", t2.size()

sphvol = camvtk.Sphere(center=(svol.center.x,svol.center.y,svol.center.z), radius=svol.radius)
sphvol.SetColor(camvtk.blue)
sphvol.SetWireframe()
sphvol.SetOpacity(0.3)
myscreen.addActor(sphvol)


drawTree(myscreen,t,opacity=0.3, color=camvtk.red)

#printNodes(t)


#printNodes(t)
#myscreen.render()
#time.sleep(1)
drawTree(myscreen,t,opacity=1, color=camvtk.green)

drawTree(myscreen,t2,opacity=1, color=camvtk.red)



print "sum total: t + t2 = ", t.size()+t2.size()
print " calling sum()"
t2.diff(t)

print " AFTER sum()"
print " sum t2.sum(t)=", t2.size()



#drawTree(myscreen,t,opacity=1)
#drawTree(myscreen,t2,opacity=0.3, color=camvtk.green)

drawTree(myscreen,t2,opacity=1, color=camvtk.blue,offset=(0,15,0))





myscreen.render()
myscreen.iren.Start() 
    
