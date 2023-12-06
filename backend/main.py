from fastapi import FastAPI, Request
from fastapi.responses import HTMLResponse
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates
from fastapi.middleware.cors import CORSMiddleware
from typing import Annotated
import requests
import time

import csv 


with open('FlowerDatabase.csv', 'r', encoding='utf-8') as file:
    csv_reader = csv.DictReader(file)
    flowers_dataset = []
    for row in csv_reader:
        flowers_dataset.append(row)
    

app = FastAPI()

app.mount('/static', StaticFiles(directory='static', html=True), name='static')

templates = Jinja2Templates(directory="templates")

planta_escolhida = None

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.get('/')
def index():
    return 'See /{id}'

@app.get('/get/dados/')
def index():
    dados_string = []
    response = requests.get('http://192.168.96.241/get')
    dados_string = response.content.decode("utf-8").split(',')
    temperatura = int(dados_string[0]) if dados_string[0] else 0
    luz = int(dados_string[1]) if dados_string[1] else 0
    agua = int(dados_string[2]) if dados_string[2] else 0

    felicidade_luz = "neutro"
    if planta_escolhida:
        felicidade_luz = "feliz"
        if luz < 600 and ("Shade" not in planta_escolhida.get("SunNeeds")):
            felicidade_luz = "triste"
        if luz > 600 and luz <= 2000 and ("Partial sun" not in planta_escolhida.get("SunNeeds")):
            felicidade_luz = "triste"
        if luz > 2000 and ("Full sun" not in planta_escolhida.get("SunNeeds")):
            felicidade_luz = "triste"
    
    felicidade_agua = "neutro"
    if planta_escolhida:
        felicidade_agua = "feliz"
        if agua >= 2000 and ("avarage" not in planta_escolhida.get("WaterNeeds")):
            felicidade_agua = "triste"
        if agua < 1000 and agua > 200 and ("low" not in planta_escolhida.get("WaterNeeds")):
            felicidade_agua = "triste"
        if agua <= 200:
            felicidade_agua = "triste"

    humor_da_planta = {
        "agua": felicidade_agua,
        "luz": felicidade_luz,
        "temperatura": "feliz" if temperatura > 16 and temperatura < 30 else "triste",
        "dados": dados_string,
        "planta_escolhida": planta_escolhida
    }
    return humor_da_planta
    
@app.get("/set/pump/")
async def read_item_by_index(request: Request):
    #print(request.headers.get('host'))
    response = requests.get('http://192.168.96.241/pump2on')
    time.sleep(10)
    response = requests.get('http://192.168.96.241/pump2off')
    
    return True

@app.get("/get/verify/")
async def verify_pump(request: Request):
    #print(request.headers.get('host'))
    response = requests.get('http://192.168.96.241/get')
    dados_string = response.content.decode("utf-8").split(',')
    if len(dados_string):
        agua = int(dados_string[2]) if dados_string[2] else 0
        if agua < 200:
            response = requests.get('http://192.168.96.241/pump2on')
            time.sleep(10)
            response = requests.get('http://192.168.96.241/pump2off') 
    
    return True

@app.get("/plant/{index}/")
async def read_item_by_index(request: Request, index: int):
    #print(request.headers.get('host'))
    global planta_escolhida 
    planta_escolhida = flowers_dataset[index]
    return flowers_dataset[index]

@app.get("/{id}", response_class=HTMLResponse)
async def read_item(request: Request, id: str):
    listFlowers = []
    for item in flowers_dataset:
        listFlowers.append({
            'Name': item['Name'],
            'Description': item['Desc']
        })
    return templates.TemplateResponse("./page.html", 
    {"request": request, 
    "tempAnterior": id, 
    "host": request.headers.get('host'),
    "listFlowers": enumerate(listFlowers)
    })

