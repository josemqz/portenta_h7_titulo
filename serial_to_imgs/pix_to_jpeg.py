from matplotlib import pyplot as plt
from PIL import Image
import numpy as np
from imgs import *

# num_img = 5

inp_filename = "inp"
# out_filename = "out"+str(num_img)
# out2_filename = "out"+str(num_img)+"_2"

# imagenes a np
# flip porque las imagenes vienen invertidas predeterminadamente
# puede que fuera porque tenia al reves la placa xd
#inp_np = np.flip(np.uint8(inp))
#out_np = np.flip(np.uint8(out))

# almacenar imagenes
inp_im = Image.fromarray((np.uint8(inp)), 'L')
inp_im.save(inp_filename + ".jpeg")

"""
out_im = Image.fromarray((np.uint8(out)), 'L')
out_im.save(out_filename + ".jpeg")

out2_im = Image.fromarray((np.uint8(out2)), 'L')
out2_im.save(out2_filename + ".jpeg")
"""

# mostrar imagenes
# plt.imshow(img_np, cmap="gray")
# plt.show()