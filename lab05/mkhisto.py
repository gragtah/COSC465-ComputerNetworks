import matplotlib                                                                       
matplotlib.use('Agg')                                                                   
import matplotlib.pyplot as plt                                                         
                                                                                        
import random                                                                           
                                                                                        
def plothisto(hlist):                                                                   
    fig = plt.figure()                                                                  
    ax = fig.add_subplot('111')                                                         
    ax.hist(hlist, histtype='bar')                                                      
    plt.xlabel('Number of BGP update messages')                                         
    plt.ylabel('Count of BGP update messages')                                          
    plt.title('Here\'s my fancy histogram!')                                            
    plt.savefig('myhistogram.pdf', bbox_inches='tight', pad_inches=0.1)               
    plt.close()                                                                         
                                                                                        
data = [ random.randint(1,100) for _ in xrange(100) ]                                   
plothisto(data)            
