import struct, os
out = r"C:\Workplace\mdldec\testdata\synthetic.mdl"
os.makedirs(os.path.dirname(out), exist_ok=True)
# header layout
# id, version, name64, length, 15 floats, then 25 int32s? count: flags,numbones,boneindex,numbonecontrollers,bonecontrollerindex,numhitboxes,hitboxindex,numseq,seqindex,numseqgroups,seqgroupindex,numtextures,textureindex,texturedataindex,numskinref,numskinfamilies,skinindex,numbodyparts,bodypartindex,numattachments,attachmentindex,soundtable,soundindex,soundgroups,soundgroupindex,numtransitions,transitionindex = 27 ints
IDSTUDIOHEADER = (ord('T')<<24)+(ord('S')<<16)+(ord('D')<<8)+ord('I')
hdr_fmt = '<ii64s i 15f 27i'
hdr_size = struct.calcsize(hdr_fmt)
tex_struct_fmt = '<64siii i'
tex_struct_size = struct.calcsize(tex_struct_fmt)
seqgroup_fmt = '<32s64sii'
seqgroup_size = struct.calcsize(seqgroup_fmt)
W=H=8
name = b'synthetic\x00'
# offsets
textureindex = hdr_size
skinindex = textureindex + tex_struct_size
seqgroupindex = skinindex + 2  # 1 short
pixindex = seqgroupindex + seqgroup_size
length = pixindex + W*H + 768
vals = [IDSTUDIOHEADER, 10, name, length,
 0.0,0.0,0.0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,  # flags
 0, hdr_size,  # numbones, boneindex (unused)
 0, hdr_size,  # controllers
 0, hdr_size,  # hitboxes
 0, hdr_size,  # numseq, seqindex
 1, seqgroupindex,  # numseqgroups, seqgroupindex
 1, textureindex, pixindex,  # numtextures, textureindex, texturedataindex
 1, 1, skinindex,  # numskinref, numskinfamilies, skinindex
 0, hdr_size,  # numbodyparts
 0, hdr_size,  # attachments
 0, hdr_size, 0, hdr_size,  # sound
 0, hdr_size]  # transitions
hdr = struct.pack(hdr_fmt, *vals)
tex = struct.pack(tex_struct_fmt, b'testtex\x00', 0, W, H, pixindex)
skin = struct.pack('<h', 0)
seqg = struct.pack(seqgroup_fmt, b'seqgroup0\x00', b'synthetic.mdl\x00', 0, 0)
pixels = bytes([i % 256 for i in range(W*H)])
palette = bytes()
for i in range(256):
    palette += bytes([i, 255-i, (i*2) % 256])
with open(out, 'wb') as f:
    f.write(hdr); f.write(tex); f.write(skin); f.write(seqg); f.write(pixels); f.write(palette)
print('wrote', out, os.path.getsize(out), 'hdr', hdr_size)
