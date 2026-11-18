# cliente_coap.py - Cliente CoAP con aiocoap
# Uso: python cliente_coap.py
# Instalación: python -m pip install aiocoap
import asyncio
from aiocoap import Context, Message, GET, POST

IP_ESP32 = "10.6.0.24"   # <-- la IP que muestra el Monitor Serie
TIMEOUT = 10                  # segundos de espera por respuesta


async def pedir(contexto, metodo, ruta, payload=b""):
    """Envía una petición y devuelve (código, payload) o None si falla."""
    peticion = Message(code=metodo, uri=f"coap://{IP_ESP32}{ruta}", payload=payload)
    try:
        respuesta = await asyncio.wait_for(
            contexto.request(peticion).response, timeout=TIMEOUT
        )
        return respuesta.code, respuesta.payload.decode(errors="replace")
    except asyncio.TimeoutError:
        print(f"  Sin respuesta en {TIMEOUT} s (la petición no llegó o la respuesta no se aceptó)")
    except Exception as e:
        print(f"  Error: {e}")
    return None


async def main():
    contexto = await Context.create_client_context()

    print("P1: GET /sensor")
    r = await pedir(contexto, GET, "/sensor")
    if r:
        print("  Código:", r[0], "| Payload:", r[1])

    print("P2: POST /led con payload ON")
    r = await pedir(contexto, POST, "/led", b"ON")
    if r:
        print("  Código:", r[0], "| Payload:", r[1])

    print("P3: GET /temperatura (se espera 4.04 Not Found)")
    r = await pedir(contexto, GET, "/temperatura")
    if r:
        print("  Código:", r[0])

    print("P4: cinco lecturas de /sensor")
    for i in range(1, 6):
        r = await pedir(contexto, GET, "/sensor")
        if r:
            print(f"  {i}: {r[1]}")

    await contexto.shutdown()


asyncio.run(main())