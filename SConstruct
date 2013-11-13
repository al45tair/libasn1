import os, os.path

env = Environment()

platform = ARGUMENTS.get('platform', None)
if platform is None:
   if os.name == 'nt':
      platform = 'win32'
   else:
      platform = 'posix'

env['CPPPATH'] = ['include', '/usr/local/botan/include/botan-1.11']
env['LIBPATH'] = ['lib', '/usr/local/botan/lib']
env['CC'] = 'clang'
env['CXX'] = 'clang++'
env['ENV']['TERM'] = os.environ['TERM']
env['CCFLAGS'] = '-g -W -Wall'
env['CXXFLAGS'] = '--std=c++11 --stdlib=libc++'

def subdirs(path):
    lst = []
    for root, dirs, files in os.walk(path):
        lst += [os.path.join(root, d) for d in dirs]
    lst.sort()
    return lst

srcdirs = subdirs('src')
srcdirs.append('src')

srcdirs += subdirs(platform)
srcdirs.append(platform)

sources = []
for d in srcdirs:
    sources.append(Glob(os.path.join(d, '*.c')))
    sources.append(Glob(os.path.join(d, '*.cc')))

env.Library('lib/asn1', sources)

for sample in Glob('samples/*.c') + Glob('samples/*.cc'):
    env.Program(sample, LIBS=['asn1', 'botan-1.11', 'c++'])

for sample in subdirs('samples'):
    sources = Glob(os.path.join(sample, '*.c')) \
              + Glob(os.path.join(sample, '*.cc'))
    env.Program(sources, LIBS=['asn1', 'botan-1.11', 'c++'])
