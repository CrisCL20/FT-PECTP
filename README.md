
# Proyecto INF-466 - Algoritmos Multiobjetivo

## Ambiente virtual

Para ejecutar los scripts que generar las instancias, es recomendable tener un ambiente virtual de python:
```
python3 -m venv venv
```

Para activar el ambiente:
```
source venv/bin/activate
```

## Crear y ejecutar instancias

Para generar el archivo de instancia (`models/instance.dat`) se usa el siguiente comando:
```
./build_instance.sh <nombre_instancia>
```

Donde `<nombre_instancia>` puede ser wbg-fal10 (instancia de prueba), pu-cs-fal07 (instancia pequeña) o pu-c8-spr07 (instancia grande).

Para ejecutar la instancia generada con AMPL se debe usar el archivo `models/runner.run`.
