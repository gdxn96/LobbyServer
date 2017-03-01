#check __init__.py if adding new models
from server import db
import datetime

class Client(db.Model):
	__tablename__ = "clients"
	id = db.Column(db.Integer, primary_key=True)
	ip_address = db.Column(db.String(20), unique=True)
	lobby_id = db.Column(db.Integer, db.ForeignKey('lobbies.id'))
	created_date = db.Column(db.DateTime, default=datetime.datetime.utcnow)

	def __repr__(self):
		return '<Client %r>' % self.id

	def to_dict(self):
		return {
			"id" : self.id,
			"ip_address" : self.ip_address,
			"lobby_id" : self.lobby_id
		}

	def to_json(self):
		return json.dumps(self.to_dict())