import math

filepath = 'Cluster_34.stl'
output_file = open("cluster.jou", "w")
output_file.write('reset\n')
with open(filepath) as fp:
    line = fp.readline()
    facet_cnt = 1
    vertex_cnt=1
    while line:
        line = fp.readline()
        if (line.find('vertex') != -1 and facet_cnt==1):
            output_file.write('create '+line.strip()+'\n')
            output_file.write('create '+fp.readline().strip()+'\n')
            output_file.write('create '+fp.readline().strip()+'\n')
        if (line.find('vertex') != -1 and facet_cnt>1):
            line = fp.readline()
            output_file.write('create '+fp.readline().strip()+'\n')
        if (line.find('vertex') != -1):
            facet_cnt+=1
        if (facet_cnt == 5):
            output_file.write('compress\n')
            for i in range(vertex_cnt,vertex_cnt+5):
                output_file.write('create curve vertex '+str(i)+' '+str(i+1)+'\n')
            output_file.write('create curve vertex '+str(vertex_cnt+5)+' '+str(vertex_cnt)+'\n')
            curve_string=''
            for i in range(vertex_cnt,vertex_cnt+6):
                curve_string+=' '+str(i)
            output_file.write('create surface curve'+curve_string+'\n')
            vertex_cnt+=6
            facet_cnt=1

output_file.write('imprint all\n')
output_file.write('merge all\n')
output_file.write('surface all size 10\n')
output_file.write('surface all scheme trimesh\n')
output_file.write('mesh surface all\n')
output_file.write('surface all smooth scheme laplacian free\n')
output_file.write('smooth surface all\n')
output_file.write('export mesh "Cluster_34.exo" overwrite')

