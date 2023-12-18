/*
Copyright © 2020 Dmytro Korniienko (kDn)
JeeUI2 lib used under MIT License Copyright (c) 2019 Marsel Akhkamov

    This file is part of FireLamp_JeeUI.

    FireLamp_JeeUI is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FireLamp_JeeUI is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FireLamp_JeeUI.  If not, see <https://www.gnu.org/licenses/>.

  (Этот файл — часть FireLamp_JeeUI.

   FireLamp_JeeUI - свободная программа: вы можете перераспространять ее и/или
   изменять ее на условиях Стандартной общественной лицензии GNU в том виде,
   в каком она была опубликована Фондом свободного программного обеспечения;
   либо версии 3 лицензии, либо (по вашему выбору) любой более поздней
   версии.

   FireLamp_JeeUI распространяется в надежде, что она будет полезной,
   но БЕЗО ВСЯКИХ ГАРАНТИЙ; даже без неявной гарантии ТОВАРНОГО ВИДА
   или ПРИГОДНОСТИ ДЛЯ ОПРЕДЕЛЕННЫХ ЦЕЛЕЙ. Подробнее см. в Стандартной
   общественной лицензии GNU.

   Вы должны были получить копию Стандартной общественной лицензии GNU
   вместе с этой программой. Если это не так, см.
   <https://www.gnu.org/licenses/>.)
*/

#ifndef __TEXT_RES_H
#define __TEXT_RES_H
#include <Arduino.h>

// Здесь строковые константы общего назначения, включая константы из интерфейса, что позволит локализировать при необходимости
// Ничего иного в данном файле не предполагается

/** набор строк с именами эффектов
 *  ссылки на эти строки собираются во флеш-массив в файле effect_types.h
 */
static constexpr const char* TEFF_000 = "";  // "пустышка"
static constexpr const char* TEFF_001  = "Lámpara blanca";
static constexpr const char* TEFF_002  = "Colores";
static constexpr const char* TEFF_003  = "Arcoiris 2D";
static constexpr const char* TEFF_004  = "Confeti";
static constexpr const char* TEFF_005  = "M&M`s";
static constexpr const char* TEFF_006  = "Ventisca, Estrellas fugaces, Meteoritos";
static constexpr const char* TEFF_007  = "Nuevo Matriz";
static constexpr const char* TEFF_008  = "Acuarela";
static constexpr const char* TEFF_009  = "Luciérnagas con rastro";
static constexpr const char* TEFF_010  = "Cubo errante";
static constexpr const char* TEFF_011  = "Pulso";
static constexpr const char* TEFF_012  = "Efecto-caida";
static constexpr const char* TEFF_013  = "Zarza ardiente";
static constexpr const char* TEFF_014  = "Paintball";
static constexpr const char* TEFF_015  = "Efectos de Ruido 3D";
static constexpr const char* TEFF_016  = "Celdas y mas";
static constexpr const char* TEFF_017  = "Quince Fichas";
static constexpr const char* TEFF_018  = "Tixy Land";
static constexpr const char* TEFF_019  = "N.Tesla";
static constexpr const char* TEFF_020  = "Oscilador";
static constexpr const char* TEFF_021  = "Viento y Lluvia";
static constexpr const char* TEFF_022  = "Hada";
static constexpr const char* TEFF_023  = "Fuente";
static constexpr const char* TEFF_024  = "Bolas rebotando";
static constexpr const char* TEFF_025  = "Sinusoide";
static constexpr const char* TEFF_026  = "Metaesferas";
static constexpr const char* TEFF_027  = "Espiral";
static constexpr const char* TEFF_028  = "Cometas, Fuegos y mas";
static constexpr const char* TEFF_029  = "BulBulator";
static constexpr const char* TEFF_030  = "Prismata";
static constexpr const char* TEFF_031  = "Rebano y Depredador";
static constexpr const char* TEFF_032  = "Remolino";
static constexpr const char* TEFF_033  = "Deriva";
static constexpr const char* TEFF_034  = "Palomitas de maíz";
static constexpr const char* TEFF_035  = "Parpadeando";
static constexpr const char* TEFF_036  = "Radar";
static constexpr const char* TEFF_037  = "Ondas";
static constexpr const char* TEFF_038  = "Fuego 2012";
static constexpr const char* TEFF_039  = "Luces de Bengala";
static constexpr const char* TEFF_040  = "Bolas";
static constexpr const char* TEFF_041  = "ADN";
static constexpr const char* TEFF_042  = "Fuego 2018";
static constexpr const char* TEFF_043  = "Cerradura de código";
static constexpr const char* TEFF_044  = "Cubo 2D";
static constexpr const char* TEFF_045  = "Magma";
static constexpr const char* TEFF_046  = "Picasso";
static constexpr const char* TEFF_047  = "Flota Estelar";
static constexpr const char* TEFF_048  = "Banderas";
static constexpr const char* TEFF_049  = "Saltadores";
static constexpr const char* TEFF_050  = "Torbellinos";
static constexpr const char* TEFF_051  = "Estrellas";
static constexpr const char* TEFF_052  = "Deslumbramiento del agua, colores";
static constexpr const char* TEFF_053  = "Fuegos artificiales";
static constexpr const char* TEFF_054  = "Océano Pacífico";
static constexpr const char* TEFF_055  = "Alfombra";
static constexpr const char* TEFF_056  = "Ruido colorado";
static constexpr const char* TEFF_057  = "Mariposas";
static constexpr const char* TEFF_058  = "Sombras";
static constexpr const char* TEFF_059  = "Patrones";
static constexpr const char* TEFF_060  = "Flechas";
static constexpr const char* TEFF_061  = "Bolas salvajes";
static constexpr const char* TEFF_062  = "Atracción";
static constexpr const char* TEFF_063  = "Serpientes";
static constexpr const char* TEFF_064  = "Nexus";
static constexpr const char* TEFF_065  = "Laberinto";
static constexpr const char* TEFF_066  = "Colores rizados";
static constexpr const char* TEFF_067  = "Aurora boreal";
static constexpr const char* TEFF_068  = "Florece";
static constexpr const char* TEFF_069  = "Isla de Serpientes";
static constexpr const char* TEFF_070  = "Sueños de los niños";
static constexpr const char* TEFF_071  = "Metabolas";
static constexpr const char* TEFF_072  = "Lámpara de lava";
static constexpr const char* TEFF_073  = "Corredor Espacial";
static constexpr const char* TEFF_074  = "Humo";
static constexpr const char* TEFF_075  = "Lampara de Fuego";
static constexpr const char* TEFF_076  = "Espejismo";
static constexpr const char* TEFF_250  = "Reloj";
static constexpr const char* TEFF_254  = "Analizador de frecuencia";
static constexpr const char* TEFF_255  = "Osciloscopio";


// -------------------------
#define DFTINTF_00A "Efecto"
#define DFTINTF_020 "Micrófono"
#define DFTINTF_084 "Paleta"
#define DFTINTF_088 "Escala"
#define DFTINTF_0D0 "Color"
#define DFTINTF_0D1 "Paleta/Escala"
#define DFTINTF_0D2 "Efecto (0 = cíclico)"
#define DFTINTF_0D3 "Desplazamiento de color (0: cíclico)"
#define DFTINTF_0D4 "Subpíxel"
#define DFTINTF_0D5 "Cantidad"
#define DFTINTF_0D6 "Generador de color"
#define DFTINTF_0D7 "Tinte"
#define DFTINTF_0D8 "¡Tonteria!"
#define DFTINTF_0D9 "Cantidad/Color (lampara nocturna)"
#define DFTINTF_0DA "Saturación"
#define DFTINTF_0DB "Ganancia"
#define DFTINTF_0DC "Barrido"
#define DFTINTF_0DD "Onda"
#define DFTINTF_0DE "Desenfocar"
#define DFTINTF_0DF "Flutter"
#define DFTINTF_0E0 "Luz nocturna"
#define DFTINTF_0E1 "Anillos"
#define DFTINTF_0E2 "Deslumbramiento (0: Apagado)"
#define DFTINTF_0E3 "Modo"
#define DFTINTF_0E4 "Velocidad lineal"
#define DFTINTF_0E5 "Modo (1: aleatorio)"
#define DFTINTF_0E6 "Desplazamiento"
#define DFTINTF_0E7 "Cantidad/Dirección/Posición"
#define DFTINTF_0E8 "Depredador"
#define DFTINTF_0E9 "Masa"
#define DFTINTF_0EA "Velocidad (vertical)"
#define DFTINTF_0EB "Velocidad (horizontal)"
#define DFTINTF_0EC "Patrón (0: aleatorio)"
#define DFTINTF_0ED "Sombreado"
#define DFTINTF_0EE "Energía"
#define DFTINTF_0EF "Densidad"
#define DFTINTF_0F0 "Color/Rotación"
#define DFTINTF_0F1 "Tamaño/Cola (1-85-170-255)"
#define DFTINTF_0F2 "Frecuencia de inicio"
#define DFTINTF_0F3 "Rotar"
#define DFTINTF_0F4 "Paleta (0: generador)"
#define DFTINTF_0F5 "Suavizado"
#define DFTINTF_0F6 "Relleno"
#define DFTINTF_0F7 "Espesor"
#define DFTINTF_0F9 "Comportamiento: 1/2"
#define DFTINTF_0FA "Color: 1/2"
#define DFTINTF_0FB "Paleta (0: sin color)"
#define DFTINTF_0FC "Color (1: aleatorio, 255: blanco)"
#define DFTINTF_0FD "Nevada/Starfall/Meteors"
#define DFTINTF_0FE "Cantidad"
#define DFTINTF_0FF "Tipo 1/2, Desenfoque 3/4"
#define DFTINTF_100 "Paleta (0: color aleatorio)"
#define DFTINTF_101 "Color 1 (0: cíclico)"
#define DFTINTF_102 "Nube"
#define DFTINTF_103 "Desenfocado (0: no)"
#define DFTINTF_104 "Deslizar"
#define DFTINTF_105 "Filtro"
#define DFTINTF_106 "Física"
#define DFTINTF_107 "Círculos"
#define DFTINTF_108 "Opción 1/2"
#define DFTINTF_109 "Parpadea"
#define DFTINTF_10A "Fallo"
#define DFTINTF_10B "Tormenta"
#define DFTINTF_10C "Cantidad de proyectiles"
#define DFTINTF_10D "Tamaño (ancho)"
#define DFTINTF_10E "Cambio de color (0: aleatorio)"
#define DFTINTF_10F "Tamaño (altura)"
#define DFTINTF_110 "Clásico"
#define DFTINTF_111 "Un color"
#define DFTINTF_112 "Colores inversos"
#define DFTINTF_113 "Color 2 (0: cíclico)"
#define DFTINTF_114 "Calidad / Velocidad"
#define DFTINTF_115 "Desenfocar (1-4 con viento)"
#define DFTINTF_116 "Desplazamiento sinusoidal"
#define DFTINTF_117 "Direccion (0: cíclico)"
#define DFTINTF_118 "Umbral de señal"
#define DFTINTF_11A "Color (1: cíclico)"
#define DFTINTF_24hrs "24hr"

/** набор строк с текстовыми константами интерфейса
 */
static constexpr const char* TINTF_000 = "Efectos";
static constexpr const char* TINTF_001 = "Representar texto";
static constexpr const char* TINTF_002 = "Configuracion";
static constexpr const char* TINTF_in_sel_lst = "Presencia en lista de Efectos";
static constexpr const char* TINTF_in_demo = "Presencia en lista demo";
static constexpr const char* TINTF_005 = "Copiar";
static constexpr const char* TINTF_006 = "Eliminar";
static constexpr const char* TINTF_007 = "Refrescar lista de Efectos";
static constexpr const char* TINTF_Save = "Guardar";
static constexpr const char* TINTF_009 = "Control de Lista de efectos";
static constexpr const char* TINTF_00A = DFTINTF_00A;
static constexpr const char* TINTF_exit = "Salir";
static constexpr const char* TINTF_00C = "Brillo Global";
static constexpr const char* TINTF_00D = "Brillo";
static constexpr const char* TINTF_00E = "Encender";
static constexpr const char* TINTF_00F = "Demo";
static constexpr const char* TINTF_010 = "Brillo.Glob";
static constexpr const char* TINTF_011 = "Eventos"; 
static constexpr const char* TINTF_012 = "Micro.";
static constexpr const char* TINTF_013 = "Boton";
static constexpr const char* TINTF_014 = "Mas...";
static constexpr const char* TINTF_015 = "<<<";
static constexpr const char* TINTF_016 = ">>>";
static constexpr const char* TINTF_017 = "En espera...";
static constexpr const char* TINTF_018 = "Configuracion";
static constexpr const char* TINTF_019 = "Bajar";
static constexpr const char* TINTF_01A = "Configuracion";
static constexpr const char* TINTF_01B = "Crear";
static constexpr const char* TINTF_01C = "Mostrar en Lampara"; 
static constexpr const char* TINTF_01D = "Texto";
static constexpr const char* TINTF_01E = "Color de Texto";
static constexpr const char* TINTF_01F = "Enviar";
static constexpr const char* TINTF_020 = DFTINTF_020;
static constexpr const char* TINTF_021 = "Micrófono Activado";
static constexpr const char* TINTF_022 = "Correcion del Zero"; 
static constexpr const char* TINTF_023 = "Nivel de Ruido, und";
static constexpr const char* TINTF_024 = "Cancelacion de ruido";
static constexpr const char* TINTF_025 = "Calibrar micrófono";
static constexpr const char* TINTF_026 = "Encende el micrófono";
static constexpr const char* TINTF_027 = "... En proceso ...";
static constexpr const char* TINTF_028 = "WiFi";
static constexpr const char* TINTF_029 = "Cliente (STA)";
static constexpr const char* TINTF_02A = "Conficuracion de Cliente WiFi";
static constexpr const char* TINTF_02B = "Nombre de Lampara (mDNS Hostname/AP-SSID)";
static constexpr const char* TINTF_02C = "WiFi SSID";
static constexpr const char* TINTF_02D = "Password";
static constexpr const char* TINTF_02E = "Conectarse";
static constexpr const char* TINTF_02F = "Punto de Acceso (PA)";
static constexpr const char* TINTF_030 = "Muestra"; 
static constexpr const char* TINTF_031 = "Configuracion WiFi & Punto de Acceso";
static constexpr const char* TINTF_032 = "En modo PA la lampara estara en modo punto de acceso y no se conectara a otras redes WiFI";
static constexpr const char* TINTF_033 = "Modo WiFi";
static constexpr const char* TINTF_034 = "Proteger PA con contraseña";
static constexpr const char* TINTF_035 = "MQTT";
static constexpr const char* TINTF_036 = "MQTT host";
static constexpr const char* TINTF_037 = "MQTT port";
static constexpr const char* TINTF_038 = "User";
static constexpr const char* TINTF_039 = "Intervalo de mqtt, sec.";
static constexpr const char* TINTF_03A = "Coneccion";
static constexpr const char* TINTF_03B = "Reflejo H";
static constexpr const char* TINTF_03C = "Reflejo V";
static constexpr const char* TINTF_03D = "Cambio de efectos sueave";
static constexpr const char* TINTF_03E = "Efecto aleatorio en Demo";
static constexpr const char* TINTF_03F = "Cambio de efecto en Demo. (sec)";
static constexpr const char* TINTF_040 = "Orden en lista de Efectos";
static constexpr const char* TINTF_041 = "Copias bajo original";
static constexpr const char* TINTF_042 = "Copias al final";
static constexpr const char* TINTF_043 = "Ordenar por índice"; 
static constexpr const char* TINTF_044 = "Velocidad del texto";
static constexpr const char* TINTF_045 = "Posición del texto";
static constexpr const char* TINTF_046 = "Mezclado (PA+STA)"; 
static constexpr const char* TINTF_047 = "Nunca"; 
static constexpr const char* TINTF_048 = "Cada hora";
static constexpr const char* TINTF_049 = "Cada media hora";
static constexpr const char* TINTF_04A = "Cada 15 minutos";
static constexpr const char* TINTF_04B = "Cada 10 minutos";
static constexpr const char* TINTF_04C = "Cada 5 minutos";
static constexpr const char* TINTF_04D = "Cada minuto";
static constexpr const char* TINTF_04E = "Felicitacion de Año Nuevo"; 
static constexpr const char* TINTF_04F = "Periodo de visualización (0 - No)";
static constexpr const char* TINTF_050 = "Fecha/Tiempo de Año Nuevo en formato YYYY-MM-DDThh:mm";
static constexpr const char* TINTF_051 = "Fecha / Hora / Zona horaria";
static constexpr const char* TINTF_052 = "Las reglas de cambio de zona horaria / horario de verano se aplican automáticamente, sin necesidad de ajustarlas manualmente.";
static constexpr const char* TINTF_053 = "Zona horaria";
static constexpr const char* TINTF_054 = "Servidor NTP de emergancia (Opcional)";
static constexpr const char* TINTF_055 = "Fecha/Hora en formato YYYY-MM-DDThh:mm:ss (si no hay conexion)";
static constexpr const char* TINTF_056 = "Actualizacion";
static constexpr const char* TINTF_058 = "Empezar";
static constexpr const char* TINTF_059 = "Fichero FirmWare";
static constexpr const char* TINTF_05A = "Subida";
static constexpr const char* TINTF_05B = "Eventos";
static constexpr const char* TINTF_05C = "Redactar";
static constexpr const char* TINTF_05D = "Añadir";
static constexpr const char* TINTF_05E = "Activado";
static constexpr const char* TINTF_05F = "Tipo de Evento";
static constexpr const char* TINTF_060 = "Encender Lampara";
static constexpr const char* TINTF_061 = "Apagar Lampara";
static constexpr const char* TINTF_062 = "Modo DEMO";
static constexpr const char* TINTF_063 = "Despertador";
static constexpr const char* TINTF_064 = "Cargar Config. Lampara";
static constexpr const char* TINTF_065 = "Cargar Config. Efectos";
static constexpr const char* TINTF_066 = "Cargar Config. Eventos";
static constexpr const char* TINTF_067 = "Mostrar texto";
static constexpr const char* TINTF_068 = "Mostrar hora";
static constexpr const char* TINTF_069 = "Estado de Pin";
static constexpr const char* TINTF_06A = "Encender AUX";
static constexpr const char* TINTF_06B = "Apagar AUX";
static constexpr const char* TINTF_06C = "Cambiar al revés AUX";
static constexpr const char* TINTF_06D = "Fecha/Hora de Evento";
static constexpr const char* TINTF_06E = "Repetir, Min";
static constexpr const char* TINTF_06F = "Parar, Min";
static constexpr const char* TINTF_070 = "Parametros (Texto)";
static constexpr const char* TINTF_071 = "Repetir";
static constexpr const char* TINTF_072 = "Lunes";
static constexpr const char* TINTF_073 = "Martes";
static constexpr const char* TINTF_074 = "Miercoles";
static constexpr const char* TINTF_075 = "Jueves";
static constexpr const char* TINTF_076 = "Viernes";
static constexpr const char* TINTF_077 = "Sabado";
static constexpr const char* TINTF_078 = "Domingo";
static constexpr const char* TINTF_079 = "Actualizar";
static constexpr const char* TINTF_07A = "Accion"; 
static constexpr const char* TINTF_07B = "Boton activado";
static constexpr const char* TINTF_07C = "OFF/ON";
static constexpr const char* TINTF_07D = "Retener"; 
static constexpr const char* TINTF_07E = "Presionar"; 
static constexpr const char* TINTF_07F = "Una vez"; 
static constexpr const char* TINTF_080 = "Lampara de Fuego"; 
static constexpr const char* TINTF_082 = "Otros";
static constexpr const char* TINTF_083 = "Cambiando, no superponer efectos";
static constexpr const char* TINTF_084 = DFTINTF_084;
static constexpr const char* TINTF_085 = "Alfabético (interno)";
static constexpr const char* TINTF_086 = "Eventos Activado ";
static constexpr const char* TINTF_087 = "Velocidad";
static constexpr const char* TINTF_088 = DFTINTF_088;
static constexpr const char* TINTF_effrename = "Nombre de efecto";
static constexpr const char* TINTF_08A = "Alfabético (Configuracion)";
static constexpr const char* TINTF_08B = "Restablecer la configuración del efecto";
static constexpr const char* TINTF_08C = "Prefijo de Topic";
static constexpr const char* TINTF_08D = "Ordenar por micrófono";
static constexpr const char* TINTF_08E = "Depuración";
static constexpr const char* TINTF_08F = "Configuracion ESP";
static constexpr const char* TINTF_090 = "Numeracion de efectos en la lista";
static constexpr const char* TINTF_091 = "Símbolo de micrófono en la lista";
static constexpr const char* TINTF_092 = "Configuracion de Pin";
static constexpr const char* TINTF_093 = "Mostrar Configuracion del Sistema";
static constexpr const char* TINTF_094 = "Pin de Boton";
//static constexpr const char* TINTF_095 = "Limite de corriente (mA)";
static constexpr const char* TINTF_096 = "Reiniciar";
static constexpr const char* TINTF_097 = "Pin RX Reproductor";
static constexpr const char* TINTF_098 = "Pin TX Reproductor";
static constexpr const char* TINTF_099 = "DFPlayer";
static constexpr const char* TINTF_09A = "Nombre de Efecto en Demo";
static constexpr const char* TINTF_09B = "Volumen";
static constexpr const char* TINTF_09C = "Tiempo";
static constexpr const char* TINTF_09D = "Nombre de efecto";
static constexpr const char* TINTF_09E = "Efecto(reset)";
static constexpr const char* TINTF_09F = "Desconectado";
static constexpr const char* TINTF_0A0 = "Primero";
static constexpr const char* TINTF_0A1 = "Aleatorio";
static constexpr const char* TINTF_0A2 = "MP3 Aleatorio";
static constexpr const char* TINTF_0A3 = "Sonido del Despertador";
static constexpr const char* TINTF_0A4 = "Segundo";
static constexpr const char* TINTF_0A5 = "Tercero";
static constexpr const char* TINTF_0A6 = "Cuarto";
static constexpr const char* TINTF_0A7 = "Quinto";
static constexpr const char* TINTF_0A8 = "Ecualizador";
static constexpr const char* TINTF_0A9 = "Normal";
static constexpr const char* TINTF_0AA = "Pop";
static constexpr const char* TINTF_0AB = "Rock";
static constexpr const char* TINTF_0AC = "Jass";
static constexpr const char* TINTF_0AD = "Clasico";
static constexpr const char* TINTF_0AE = "Bass";
static constexpr const char* TINTF_0AF = "Reproductor MP3";
static constexpr const char* TINTF_0B0 = "Cantidad de ficheros en carpeta MP3";
static constexpr const char* TINTF_0B1 = "Modo de reproduccion";
static constexpr const char* TINTF_0B2 = "Carpeta\\Numero de fichero musical(ej.: MP3\\17, 5\\19)";
static constexpr const char* TINTF_0B3 = "Limitar volumen del Despertador";
static constexpr const char* TINTF_0B4 = "Por defecto";
static constexpr const char* TINTF_0B5 = "Eliminar de la lista";
static constexpr const char* TINTF_0B6 = "Desconectado";
static constexpr const char* TINTF_0B7 = "Ejemplo 1";
static constexpr const char* TINTF_0B8 = "Ejemplo 2";
static constexpr const char* TINTF_0B9 = "Parametro";
static constexpr const char* TINTF_0BA = "Despertador Amanecer";
static constexpr const char* TINTF_0BB = "Duración del Amanecer";
static constexpr const char* TINTF_0BC = "Brillar después del amanecer";
static constexpr const char* TINTF_0BD = "<";
static constexpr const char* TINTF_0BE = ">";
static constexpr const char* TINTF_0BF = "<+5";
static constexpr const char* TINTF_0C0 = "+5>";
static constexpr const char* TINTF_0C1 = "segundos";
static constexpr const char* TINTF_0C2 = "minutos";
static constexpr const char* TINTF_0C3 = "horas";
static constexpr const char* TINTF_0C4 = "dias";
static constexpr const char* TINTF_0C5 = "dias";
static constexpr const char* TINTF_0C6 = "día";
static constexpr const char* TINTF_0C7 = "horas";
static constexpr const char* TINTF_0C8 = "hora";
static constexpr const char* TINTF_0C9 = "El Azar.";
static constexpr const char* TINTF_0CA = "Oscurecer el fondo";
static constexpr const char* TINTF_0CB = "Advertencia";
static constexpr const char* TINTF_0CC = "minutos";
static constexpr const char* TINTF_0CD = "minuto";
static constexpr const char* TINTF_0CE = "Dibujo";
static constexpr const char* TINTF_0CF = "Llenar";
static constexpr const char* TINTF_0D0 = DFTINTF_0D0;
static constexpr const char* TINTF_0D1 = "Sonido primero";
static constexpr const char* TINTF_0D2 = "Limita el volumen";
static constexpr const char* TINTF_0D3 = "Coef. velocidad (1.0 por defecto)";
static constexpr const char* TINTF_0D4 = "Configuraciones de pantalla";
//static constexpr const char* TINTF_0D5 = "Brillo (0 - OFF)";
//static constexpr const char* TINTF_0D6 = "Brillo con OFF lámpara (0 - OFF)";
//static constexpr const char* TINTF_0D7 = "Formato de 24 horas";
//static constexpr const char* TINTF_0D8 = "Líder 0";
static constexpr const char* TINTF_0D9 = "Claro";
static constexpr const char* TINTF_0DA = "Escanear";
static constexpr const char* TINTF_0DB = "FTP";
static constexpr const char* TINTF_0DC = "Control";
static constexpr const char* TINTF_0DD = "Tipo de escala";
static constexpr const char* TINTF_0DE = "Escala color";
static constexpr const char* TINTF_0DF = "Texto color";
static constexpr const char* TINTF_0E0 = "Mostrar temperatura";
static constexpr const char* TINTF_0E1 = "¿Está seguro?";
static constexpr const char* TINTF_0E2 = "Transmisión";
static constexpr const char* TINTF_0E3 = "Modo";
static constexpr const char* TINTF_0E4 = "E1.31";
static constexpr const char* TINTF_0E5 = "SoulMate";
static constexpr const char* TINTF_0E6 = "Efecto bloque";
static constexpr const char* TINTF_0E7 = "Mapang";
static constexpr const char* TINTF_0E8 = "Universo primero";
static constexpr const char* TINTF_0E9 = "Cargar Config. Botons";
static constexpr const char* TINTF_0EA = "Luz de noche";
static constexpr const char* TINTF_0EB = "Lámpara brillante";
static constexpr const char* TINTF_0EC = "De todos modos";
static constexpr const char* TINTF_0ED = "Dice el tiempo";
static constexpr const char* TINTF_0EE = "Discapacitada";
static constexpr const char* TINTF_0EF = "Vertical";
static constexpr const char* TINTF_0F0 = "Horizontal";

static constexpr const char* TINTF_display_setup = "LED-display setup";
static constexpr const char* TINTF_display_type = "LED matrix type";
static constexpr const char* TINTF_f_restore_state = "Restore state on power-on";
static constexpr const char* TINTF_cfg_hub75 = "Конфигурация HUB75 панелей";
static constexpr const char* TINTF_gpiocfg = "GPIO Configuration";
static constexpr const char* TINTF_ledstrip = "LED strip layout";
static constexpr const char* TINTF_setup_tm1637 = "TM1637 Display";

static constexpr const char* NY_MDG_STRING1 = "Hasta que se quede el año nuevo %d %s";
static constexpr const char* NY_MDG_STRING2 = "Feliz año nuevo %d!";
//-------------------------

#endif