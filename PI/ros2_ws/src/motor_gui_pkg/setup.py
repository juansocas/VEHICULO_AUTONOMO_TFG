from setuptools import find_packages, setup

package_name = 'motor_gui_pkg'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        ('share/' + package_name + '/launch', ['launch/motor_gui_launch.py']),
    ],
    install_requires=['setuptools'],  # Solo si realmente necesitas setuptools aquí
    zip_safe=True,
    maintainer='jsc',
    maintainer_email='juan.socas101@alu.ulpgc.es',
    description='TODO: Package description',
    license='TODO: License declaration',
    entry_points={
        'console_scripts': [
            'motor_gui = motor_gui_pkg.motor_gui:main',
        ],
    },
)
