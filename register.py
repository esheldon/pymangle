import os

rname='README.md'
rstname='README.rst'

def clear():
    if os.path.exists(rstname):
        os.remove(rstname)

def try_make_rst():
    cmd="pandoc --from=markdown --to=rst {readme} -o {rst}"
    cmd=cmd.format(readme=rname, rst=rstname)
    os.system(cmd)

def register():
    os.system("python setup.py register")

clear()

try_make_rst()
register()

clear()
