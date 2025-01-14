# Automatización del posicionamiento y seguimiento de trayectorias de un vehículo autónomo basado en ROS sobre planos previamente levantados

![image](https://github.com/user-attachments/assets/bb9f78dd-d86d-4677-ab99-524ed244fb90)


## Resumen del Proyecto

Este Trabajo Fin de Grado se centra en el desarrollo de un **vehículo autónomo** capaz de realizar **levantamiento de mapas** y **navegación autónoma**, empleando el entorno de desarrollo **ROS 2** y tecnologías avanzadas como **sensores LIDAR**. El diseño del sistema integra módulos clave, como **percepción**, **localización**, **control de movimiento** y **navegación**, logrando una interacción eficiente entre el hardware y el software del vehículo.

En la **primera fase del proyecto**, se implementaron algoritmos **SLAM** para la creación precisa de mapas del entorno. Estos algoritmos, combinados con el sensor LIDAR, permitieron capturar información detallada de las características estructurales y puntuales del entorno, estableciendo una base sólida para la navegación autónoma. En la **segunda fase**, se configuraron y ejecutaron herramientas como **Nav2**, que permitieron al vehículo optimizar rutas en tiempo real, esquivar obstáculos detectados de forma dinámica y adaptarse a los cambios en el entorno.

El vehículo autónomo fue evaluado mediante diversas pruebas en escenarios controlados, demostrando su capacidad para generar mapas precisos y ejecutar trayectorias planificadas con alta exactitud. Los resultados confirmaron que el diseño e integración del sistema permitieron al vehículo desempeñar sus funciones de manera eficaz, validando el éxito del desarrollo tanto a nivel conceptual como práctico.

## Project Summary

This TFG focuses on the development of an **autonomous vehicle** capable of performing **map generation** and **autonomous navigation**, utilizing the **ROS 2 development environment** and advanced technologies such as **LIDAR sensors**. The system integrates key modules, including **perception**, **localization**, **motion control**, and **navigation**, achieving efficient interaction between the vehicle's hardware and software.

In the **first phase of the project**, **SLAM algorithms** were implemented for the precise creation of environmental maps. These algorithms, combined with the LIDAR sensor, enabled detailed capture of structural and specific features of the environment, establishing a robust foundation for autonomous navigation. In the **second phase**, tools such as **Nav2** were configured and executed, allowing the vehicle to optimize routes in real time, avoid dynamically detected obstacles, and adapt to environmental changes.

The autonomous vehicle was evaluated through various tests in controlled scenarios, demonstrating its ability to generate accurate maps and execute planned trajectories with high precision. The results confirmed that the system's design and integration enabled the vehicle to perform its functions effectively, validating the project's success both conceptually and practically.

---

## Detalles del Proyecto

Este proyecto está desarrollado para **ROS 2 Humble**, utilizando **Ubuntu Server 22.04** en la Raspberry Pi y **Ubuntu Desktop 22.04** en el portátil.

### Estructura del Proyecto

El proyecto está organizado en tres directorios principales:

- **PC**: Contiene el espacio de trabajo correspondiente al portátil. Aquí se configuran y ejecutan los nodos necesarios para la supervisión, visualización y control del vehículo, como **RVIZ** y herramientas de planificación.
- **PI**: Incluye el espacio de trabajo de la Raspberry Pi. Este directorio contiene los nodos responsables de la comunicación con el hardware del vehículo, como sensores y actuadores, y de la ejecución local del control del movimiento.
- **ARDUINO**: Este directorio está dedicado al código que se ejecuta en la placa Arduino, encargado de controlar directamente los motores y recibir los comandos de velocidad desde la Raspberry Pi.

Cada directorio funciona como un espacio de trabajo independiente, diseñado para su respectivo componente del sistema: el portátil, la Raspberry Pi o el Arduino. Esto permite una integración modular y una gestión eficiente del código en cada entorno.

### Requisitos

- **ROS 2 Humble** instalado en el portátil y la Raspberry Pi.
- **Ubuntu Server 22.04** en la Raspberry Pi.
- **Ubuntu Desktop 22.04** en el portátil.
- Herramientas adicionales como `colcon` para la compilación y manejo de espacios de trabajo.

### Objetivo

Automatizar el posicionamiento y seguimiento de trayectorias de un vehículo autónomo utilizando ROS 2 sobre planos previamente levantados.

---

Este diseño modular facilita la implementación, depuración y extensión del sistema, garantizando un flujo eficiente de datos entre los componentes y permitiendo futuras mejoras en el proyecto.
