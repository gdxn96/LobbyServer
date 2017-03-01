#check __init__.py if adding new models
from server import db
import json
import datetime

class Lobby(db.Model):
	__tablename__ = "lobbies"
	id = db.Column(db.Integer, primary_key=True)
	num_players = db.Column(db.Integer)
	max_players = db.Column(db.Integer)
	clients = db.relationship('Client', backref='lobby',
										foreign_keys='Client.lobby_id',
										lazy='dynamic')
	created_date = db.Column(db.DateTime, default=datetime.datetime.utcnow)
	host_id = db.Column(db.Integer, db.ForeignKey('clients.id'))
	host = db.relationship("Client", uselist=False, foreign_keys='Lobby.host_id')

	def __init__(self, max_players, num_players=0):
		self.max_players = max_players
		self.num_players = num_players

	def __repr__(self):
		return '<Lobby %r>' % self.id

	def to_dict(self):
		return {
			"id" : self.id,
			"num_players" : self.num_players,
			"max_players" : self.max_players
		} 

	def to_json(self):
		return json.dumps(self.to_dict())