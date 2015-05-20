from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

ext = Extension("peval_ex",
                sources=['peval_ex.pyx'],
                language='c++',
                libraries=['peval'])
setup(ext_modules=[ext],
      cmdclass={'build_ext': build_ext})
